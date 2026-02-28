### Docker Helpfile

#### Installing Docker Compose

There is a file caller `docker-compose.yml` in the root of project. This enables one commmand `make` to fire up docker container and run the various build targets. The `docker-compose.yml` in project root, allowing you to run all build targets with a single `make` command.

#### Prerequisites (tested with Linux Mint and Pop_OS)

Add Dockers official GPG key

```bash
sudo apt update
sudo apt install ca-certificates curl gnupg lsb-release
sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
sudo chmod a+r /etc/apt/keyrings/docker.gpg
```

Download Docker

```bash
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] \
  https://download.docker.com/linux/ubuntu \
  noble stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```

#### Install Compose Pluging

```bash
sudo apt update
sudo apt install docker-ce docker-ce-cli containerd.io docker-compose-plugin
```

#### Verify Compose Install

```bash
docker compose version
```

#### Docker Compose Commands

```bash
docker compose down             # Shutdown Docker Container
docker compose up -d --build    # Build Docker Container
docker compose build --no-cache # Full Rebuild Docker Container
docker compose up -d            # Startup Docker Container
```

>**NOTE:** Review [Makefile Targets](../Makefile)

#### Docker user Group Linux

>**NOTE:** **_WARNING_**: Users in Docker Group have root access

```bash
# Add $USER to docker group
sudo usermod -aG docker $USER
newgrp docker
```

Then **log out and back in** (or sudo reboot) for changes to take effect.