#!/bin/bash

CONFIG=${1:-debug}
PLATFORM=${2:-linux}

echo "Building ${PLATFORM}_${CONFIG}"

docker run --rm \
  -v $(pwd):/workspace \
  -w /workspace \
  r36s-build \
  make -f Makefile.mk PLATFORM=${PLATFORM} CONFIG=${CONFIG}

if [ $? -ne 0 ]; then
    echo "Build is busted"
    exit 1
fi

echo "Starting slilab"

export DISPLAY=${DISPLAY:-:0}

docker run --rm \
  --gpus all \
  -e DISPLAY=$DISPLAY \
  -e WAYLAND_DISPLAY=$WAYLAND_DISPLAY \
  -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR \
  -e PULSE_SERVER=$PULSE_SERVER \
  -v /tmp/.X11-unix:/tmp/.X11-unix:rw \
  -v /mnt/wslg:/mnt/wslg \
  -v $(pwd):/workspace \
  -w /workspace \
  r36s-build \
  ./build/${PLATFORM}_${CONFIG}/target_${PLATFORM}
