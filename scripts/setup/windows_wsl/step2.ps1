# ============================================================
# Step 2 Docker + Compose + Repo build + Web launch
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

# Wait for WSL to be ready
Wait-WSLReady

Step 'Verifying labuser exists'
$userCheck = wsl -d Ubuntu -- bash -c 'id labuser &>/dev/null && echo "exists" || echo "missing"'
if ($userCheck.Trim() -ne 'exists') {
    Fail 'labuser does not exist. Did you complete Step 1 and run "wsl --shutdown"?'
}

Step 'Checking that systemd is active in Ubuntu'
$init = wsl -d Ubuntu -- bash -c 'ps -p 1 -o comm='
if ($init.Trim() -ne 'systemd') {
    Fail 'systemd is NOT active. Run "wsl --shutdown" after Phase 1, then reopen Ubuntu.'
}

Step 'Installing Docker Engine inside Ubuntu'
$dockerCmd = 'set -e && apt-get update -y && apt-get install -y ca-certificates curl gnupg lsb-release && curl -fsSL https://get.docker.com | sh && systemctl enable docker && systemctl start docker && usermod -aG docker labuser'
wsl -d Ubuntu --user root -- bash -c "$dockerCmd"

Step 'Installing Docker Compose v2'
wsl -d Ubuntu --user root -- bash -c 'apt-get update -y && apt-get install -y docker-compose-plugin'

Step 'Cloning repo and building Docker container'
$repoCmd = 'set -e && apt-get install -y git make build-essential && REPO_DIR="/home/labuser/r36s" && if [ ! -d "$REPO_DIR" ]; then sudo -u labuser git clone -b cpp https://bitbucket.org/MuddyGames/r36s.git "$REPO_DIR"; else cd "$REPO_DIR" && sudo -u labuser git fetch origin && sudo -u labuser git checkout cpp && sudo -u labuser git pull origin cpp; fi && cd "$REPO_DIR" && sudo -u labuser make release_web'
wsl -d Ubuntu --user root -- bash -c "$repoCmd"

Step 'Setup complete: opening web browser'
Write-Host ''
Write-Host 'WSL Ubuntu + Docker Engine + Docker Compose + Repo Build Complete' -ForegroundColor Green
Write-Host ''
Write-Host 'NEXT STEPS:'
Write-Host '1. (Optional) Restart WSL: wsl --shutdown'
Write-Host '2. Open Ubuntu'
Write-Host '3. Verify: docker version, docker compose version, docker run hello-world'
Write-Host '4. Repo is at /home/labuser/r36s'
Write-Host ''
Start-Process 'http://localhost:8080/gpp_web.html'