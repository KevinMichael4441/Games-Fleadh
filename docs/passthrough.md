### GPU Passthrough

When adding windows build GPU needs to be passed through to docker container to display the exe via wine.

NVIDIA: NVIDIA Container Toolkit on host
AMD GPU / iGPU / Intel : Mesa + /dev/dri

#### Checking Display

```bash
echo "$DISPLAY"
ls -l /tmp/.X11-unix
```

#### Check GPU

NVIDIA
```bash
nvidia-smi
```

AMD(Mesa)
```bash
ls -l /dev/dri
```

#### Allow use of GPU

```bash
#enable
xhost +si:localuser:root

#disable
xhost -si:localuser:root
```

#### Installing NVIDIA Runtime (Linux)

```bash
sudo apt-get update
sudo apt-get install -y ca-certificates curl gnupg

sudo install -m 0755 -d /etc/apt/keyrings
curl -fsSL https://nvidia.github.io/libnvidia-container/gpgkey | \
  sudo gpg --dearmor -o /etc/apt/keyrings/nvidia-container-toolkit.gpg
sudo chmod a+r /etc/apt/keyrings/nvidia-container-toolkit.gpg

curl -fsSL https://nvidia.github.io/libnvidia-container/stable/deb/nvidia-container-toolkit.list | \
  sed 's#deb https://#deb [signed-by=/etc/apt/keyrings/nvidia-container-toolkit.gpg] https://#g' | \
  sudo tee /etc/apt/sources.list.d/nvidia-container-toolkit.list

sudo apt-get update
sudo apt-get install -y nvidia-container-toolkit

sudo nvidia-ctk runtime configure --runtime=docker
sudo systemctl restart docker
```

```bash
docker info | grep -i runtimes
```
Run with Passthrough
```bash
docker run -it --rm \
  --gpus all \
  -e DISPLAY=:0 \
  -e NVIDIA_VISIBLE_DEVICES=all \
  -e NVIDIA_DRIVER_CAPABILITIES=graphics,display,utility,compute \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$(pwd)":/gpp \
  r36s-build
```
AMD and Others
This is already in makefile
```bash
docker run -it --rm \
  -e DISPLAY=:0 \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --device /dev/dri \
  -v "$(pwd)":/gpp \
  r36s-build
```

#### Software rendering

Inside the container (or bake into image):

```bash
make shell

apt-get update
apt-get install -y xvfb mesa-utils libgl1-mesa-dri

# Check container
glxinfo -B
```

Inside Docker Run headless Software renderer:

Xvfb :99 -screen 0 1280x720x24 &
export DISPLAY=:99
export LIBGL_ALWAYS_SOFTWARE=1
glxinfo -B

TODO Tests and tidy this up

POSSIBLY try headless only.... so no passthrough

1. If they want to see Windows exe running need to passthrough GPU
(they need to deal with NVIDIA, AMD etc)
2. Could use software renderer passthrough as a sanity check like qemu on ARM
3. Could use wine on host on Linux
4. Could run the exe on windows directly if they are already running Docker via windows


# GPU Modes: Running Game on Docker (Linux)

**Build and running games inside Docker** is perfect for Web, needs config for Linux and Windows. 
How this is achieved depends on the GPU in your machine. Specific make targets are available for R36S.

Use table below to choose **mode**. Recommend starting with **Headless mode**.

## Display Mode Guide

| Use Case                             | Recommended mode             | Game window? |
|--------------------------------------|------------------------------|--------------|
| Just want to test build              | **Headless (Software)**      | [ _ ] No     |
| Graphics setup is broken             | **Headless (Software)**      | [ _ ] No     |
| Lab machine / Library Machine        | **Headless (Software)**      | [ _ ] No     |
| Intel iGPU or AMD GPU                | **Mesa Passthrough**         | [ X ] Yes    |
| NVIDIA GPU                           | **NVIDIA Passthrough**       | [ X ] Yes    |
| Running Windows                      | **Run on Windows**           | [ X ] Yes    |
| Running Linux and need Windows build | **Wine on Linux**            | [ X ] Yes    |


> **Important:** Docker = *reproducible builds*. It does **not** mean desktop graphics on host.

## Headless (Software)

**Use this if:**

- Most reliable option
- Checking that the game builds and runs
- Don't need to see a window

**Pros**

- Works on all machines
- No GPU setup required
- No X11 permissions

**Cons**

- No visible window
- Software rendering only

### Run

```bash
docker run -it --rm \
  -v "$(pwd)":/gpp \
  r36s-build
```

Inside the container:

```bash
apt-get update
apt-get install -y xvfb

Xvfb :99 -screen 0 640x480x24 &
export DISPLAY=:99
export LIBGL_ALWAYS_SOFTWARE=1

glxinfo -B
```

## Mesa Passthrough

**Use this if:**

- PC has Intel iGPU or an AMD GPU
- Need to see a window

**Pros**

- Visible window
- No NVIDIA-specific setup

**Cons**

- Depends on host graphics configuration

### Run (host)

```bash
xhost +si:localuser:root


docker run -it --rm \
  -e DISPLAY=:0 \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --device /dev/dri \
  -v "$(pwd)":/gpp \
  r36s-build
```

Inside the container:

```bash
glxinfo -B
```
## NVIDIA Passthrough

**Use this if:**

- NVIDIA GPU
- Need to see a window

**Pros**

- Best performance
- Correct NVIDIA OpenGL

**Cons**

- Requires NVIDIA Container Toolkit
- More setup

### Host requirements

```bash
nvidia-smi
```

Docker must list the NVIDIA runtime:

```bash
docker info | grep -i runtimes
```

### Run

```bash
xhost +si:localuser:root


docker run -it --rm \
  --gpus all \
  -e DISPLAY=:0 \
  -e NVIDIA_VISIBLE_DEVICES=all \
  -e NVIDIA_DRIVER_CAPABILITIES=graphics,display,utility,compute \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  -v "$(pwd)":/gpp \
  r36s-build
```

Inside the container:

```bash
glxinfo -B
```
## Alternatives

### Run on Windows

- Using Docker on Windows
- Build in Docker
- Run `.exe` on Windows

### Wine on Linux

- Build Windows `.exe` in Docker
- Run using **Wine on the host**
- Avoids GPU passthrough