#!/usr/bin/env bash
set -euo pipefail

#=================================================================
# After some testing the only reliable way to get Docker running 
# on Windows for this project is to install 
# WSL linux distro (Ubuntu)
# Configure Docker and project container 
#=================================================================

#=================================================================
# Check docker installation location on system
#=================================================================
DOCKER_BIN="/c/Program Files/Docker/Docker/resources/bin"
mkdir -p /usr/local/bin

#=================================================================
# Symbolic links to Docker
#=================================================================
ln -sf "$DOCKER_BIN/docker.exe" /usr/local/bin/docker
ln -sf "$DOCKER_BIN/docker-compose.exe" /usr/local/bin/docker-compose

#=================================================================
# Setup credentials
#=================================================================
cat >/usr/local/bin/docker-credential-wincred <<EOF
#!/usr/bin/env bash
exec "$DOCKER_BIN/docker-credential-wincred.exe" "\$@"
EOF
chmod +x /usr/local/bin/docker-credential-wincred

hash -r

#=================================================================
# Register
#=================================================================
echo "Docker Msys2 Hooks Installed"
command -v docker docker-compose docker-credential-wincred
