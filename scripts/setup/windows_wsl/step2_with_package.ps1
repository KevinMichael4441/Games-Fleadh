# ============================================================
# Step 2 Docker + Compose + Repo clone
# ============================================================
$ErrorActionPreference = 'Stop'

function Step($msg) {
    Write-Host ''
    Write-Host "==> $msg" -ForegroundColor Cyan
}

function Fail($msg) {
    Write-Host ''
    Write-Host "ERROR: $msg" -ForegroundColor Red
    exit 1
}

function Wait-WSLReady {
    Step 'Checking WSL status'
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
    
    Fail "WSL did not become ready after $($maxAttempts * 10) seconds. Try running 'wsl --shutdown' and run this script again."
}

Wait-WSLReady

Step 'Verifying labuser exists'
$userCheck = wsl -d Ubuntu -- bash -c 'id labuser >/dev/null 2>&1 && echo exists || echo missing'
if ($userCheck.Trim() -ne 'exists') {
    Fail 'labuser does not exist. Did you complete Step 1 and run "wsl --shutdown"?'
}

Step 'Checking that systemd is active in Ubuntu'
$init = wsl -d Ubuntu -- bash -c 'ps -p 1 -o comm='
if ($init.Trim() -ne 'systemd') {
    Fail 'systemd is NOT active. Run "wsl --shutdown" after Step 1.'
}

Step 'Installing Docker Engine inside Ubuntu'
Write-Host "This may take several minutes..." -ForegroundColor Yellow
wsl -d Ubuntu --user root -- bash -c 'apt-get update -y && apt-get install -y ca-certificates curl && curl -fsSL https://get.docker.com | sh && systemctl enable docker && systemctl start docker && usermod -aG docker labuser'
Write-Host "Docker Engine installed" -ForegroundColor Green

Step 'Installing Docker Compose'
wsl -d Ubuntu --user root -- bash -c 'apt-get install -y docker-compose-plugin'
Write-Host "Docker Compose installed" -ForegroundColor Green

Step 'Installing git and build tools'
wsl -d Ubuntu --user root -- bash -c 'apt-get install -y git make build-essential'
Write-Host "Build tools installed" -ForegroundColor Green

Step 'Cloning repository'
Write-Host "Cloning from Bitbucket..." -ForegroundColor Yellow
wsl -d Ubuntu --user root -- bash -c 'sudo -u labuser git clone -b cpp https://bitbucket.org/MuddyGames/r36s.git /home/labuser/r36s'

Step 'Verifying repository exists'
$repoCheck = wsl -d Ubuntu -- bash -c 'test -d /home/labuser/r36s && echo exists || echo missing'
if ($repoCheck.Trim() -ne 'exists') {
    Fail 'Repository was not cloned successfully'
}
Write-Host "Repository cloned to /home/labuser/r36s" -ForegroundColor Green

Write-Host ''
Write-Host '=====================================================================' -ForegroundColor Green
Write-Host ' SETUP COMPLETE' -ForegroundColor Green
Write-Host '=====================================================================' -ForegroundColor Green
Write-Host ''
Write-Host 'Docker Engine: Installed' -ForegroundColor White
Write-Host 'Repository: /home/labuser/r36s' -ForegroundColor White
Write-Host ''
Write-Host 'Building the project requires a pre-built Docker image' -ForegroundColor Yellow
Write-Host 'due to corporate network SSL restrictions.' -ForegroundColor Yellow
Write-Host ''
Write-Host 'Contact instructor for r36s-build.tar.gz' -ForegroundColor Cyan
Write-Host ''