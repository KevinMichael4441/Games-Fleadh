# ============================================================
# Step 1 WSL + Ubuntu + systemd + user
# ============================================================

$ErrorActionPreference = "Stop"

function Step($msg) {
    Write-Host "`n==> $msg" -ForegroundColor Cyan
}

function Fail($msg) {
    Write-Host "`nERROR: $msg" -ForegroundColor Red
    exit 1
}

function Need-Reboot($reason) {
    Write-Host "`n=========================================" -ForegroundColor Yellow
    Write-Host " REBOOT REQUIRED" -ForegroundColor Yellow
    Write-Host " Reason: $reason" -ForegroundColor Yellow
    Write-Host " Please reboot, then re-run this script." -ForegroundColor Yellow
    Write-Host "=========================================`n" -ForegroundColor Yellow
    exit 3010
}

function Test-WSLAvailable {
    try {
        wsl.exe --status | Out-Null
        return $true
    } catch {
        return $false
    }
}

function Wait-WSLReady {
    Step "Waiting for WSL to be ready"
    $maxAttempts = 30
    $attempt = 0
    
    while ($attempt -lt $maxAttempts) {
        try {
            $status = wsl.exe --status 2>&1 | Out-String
            
            if ($status -match "WSL is finishing an upgrade|upgrade") {
                $attempt++
                Write-Host "WSL is upgrading... waiting (attempt $attempt/$maxAttempts)" -ForegroundColor Yellow
                Start-Sleep -Seconds 10
            }
            else {
                Write-Host "WSL is ready!" -ForegroundColor Green
                return $true
            }
        }
        catch {
            $attempt++
            Write-Host "Checking WSL status... (attempt $attempt/$maxAttempts)" -ForegroundColor Yellow
            Start-Sleep -Seconds 10
        }
    }
    
    Fail "WSL did not become ready after $($maxAttempts * 10) seconds. Try rebooting and running this script again."
}

function Enable-WSLFeatures {
    Step "Enabling Windows features for WSL (may require reboot)"

    $out1 = dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart
    $out2 = dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart

    $restartNeeded =
        (($out1 -join "`n") -match "Restart Needed\s*:\s*Yes") -or
        (($out2 -join "`n") -match "Restart Needed\s*:\s*Yes")

    if ($restartNeeded) {
        Need-Reboot "WSL/VirtualMachinePlatform features were enabled"
    }
}

# ------------------------------------------------------------
# Ensure wsl.exe exists at all
# ------------------------------------------------------------
Step "Checking for wsl.exe"

if (-not (Get-Command wsl.exe -ErrorAction SilentlyContinue)) {
    Fail "wsl.exe not found. This Windows build does not include WSL CLI. Update Windows, then re-run."
}

# ------------------------------------------------------------
# Enable features FIRST (fixes 'WSL is not installed' case)
# ------------------------------------------------------------
Enable-WSLFeatures

# ------------------------------------------------------------
# Preflight: WSL kernel check / update
# ------------------------------------------------------------
Step "Preflight: Checking WSL availability"

if (-not (Test-WSLAvailable)) {
    # On some systems, enabling features still needs reboot before WSL becomes usable
    Need-Reboot "WSL features enabled but WSL is not yet available"
}

Step "Updating WSL kernel (best-effort)"
try {
    wsl.exe --update | Out-Null
} catch {
    # If update isn't supported on this OS, you can ignore here and rely on manual kernel install message below
    Write-Host "WSL update command failed/unsupported on this system. Continuing..." -ForegroundColor Yellow
}

# Wait for WSL to finish any upgrade before checking kernel
Wait-WSLReady

$kernelCheck = (wsl.exe --status 2>&1) -join "`n"
if ($kernelCheck -match "requires an update to its kernel") {
    Fail (
        "WSL kernel is missing.`n`n" +
        "MANUAL STEP REQUIRED (one-time):`n" +
        "1. Download and install:`n" +
        "   https://aka.ms/wsl2kernel`n" +
        "2. Reboot (recommended)`n" +
        "3. Re-run this script"
    )
}

# ------------------------------------------------------------
# Now it's safe to touch wsl -l / unregister, etc.
# ------------------------------------------------------------
Step "Cleaning previous Ubuntu installation (if exists)"

$existing = @()
try {
    $existing = wsl.exe -l -q | Where-Object { $_ -match "^Ubuntu" }
} catch {
    # If listing distros fails for some reason, don't hard fail here
    Write-Host "Could not list WSL distros yet. Skipping cleanup." -ForegroundColor Yellow
}

if ($existing) {
    Write-Host "Unregistering existing Ubuntu distro..."
    wsl.exe --unregister Ubuntu

    $timeout = 30
    $elapsed = 0
    while ($true) {
        Start-Sleep -Seconds 2
        $elapsed += 2
        $existingCheck = wsl.exe -l -q | Where-Object { $_ -match "^Ubuntu" }
        if (-not $existingCheck) { break }
        if ($elapsed -ge $timeout) { Fail "Ubuntu still exists after unregistering." }
    }
    Write-Host "Ubuntu successfully unregistered."
}

# ------------------------------------------------------------
# Set WSL2 default (safe now)
# ------------------------------------------------------------
Step "Setting default WSL version to 2"
wsl.exe --set-default-version 2 | Out-Null

# ------------------------------------------------------------
# Install Ubuntu if missing
# ------------------------------------------------------------
Step "Installing Ubuntu (non-interactive)"

$distros = wsl.exe -l -q 2>$null
$ubuntuDistro = $distros | Where-Object { $_ -match "^Ubuntu" } | Select-Object -First 1

if (-not $ubuntuDistro) {
    Write-Host "Ubuntu not found. Installing..."
    wsl.exe --install -d Ubuntu --no-launch
    Start-Sleep -Seconds 5
}

# ------------------------------------------------------------
# Wait for Ubuntu to initialize
# ------------------------------------------------------------
Step "Waiting for Ubuntu registration"

$timeout = 60
$elapsed = 0
while ($true) {
    try {
        wsl.exe -d Ubuntu --user root -- echo "Ubuntu initialized" | Out-Null
        break
    } catch {
        Start-Sleep -Seconds 2
        $elapsed += 2
        if ($elapsed -ge $timeout) { Fail "Ubuntu failed to initialize after installation." }
    }
}
Write-Host "Ubuntu is fully registered and ready."

# ------------------------------------------------------------
# Configure systemd + default user
# ------------------------------------------------------------
Step "Configuring systemd and labuser inside Ubuntu"

$linuxSetup = @(
'set -e',
'echo "[boot]" > /etc/wsl.conf',
'echo "systemd=true" >> /etc/wsl.conf',
'echo "" >> /etc/wsl.conf',
'echo "[user]" >> /etc/wsl.conf',
'echo "default=labuser" >> /etc/wsl.conf',
'id labuser &>/dev/null || useradd -m -s /bin/bash labuser',
'echo "labuser:labuser" | chpasswd',
'usermod -aG sudo labuser'
) -join " && "

wsl.exe -d Ubuntu --user root -- bash -c "$linuxSetup"

Step "Phase 1 complete"

Write-Host "`n========================================="
Write-Host " Step 1 COMPLETE"
Write-Host "========================================="
Write-Host "Now run:"
Write-Host "    wsl --shutdown"
Write-Host "Then run Step 2:"
Write-Host "   powershell -ExecutionPolicy Bypass -NoProfile -File .\step2.ps1"
Write-Host "=========================================`n"