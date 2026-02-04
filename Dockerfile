# Most compatible linux distro (R36S/ArkOS system has an older version of GLIBC)
FROM debian:bullseye

# Prevent tzdata prompts
ENV DEBIAN_FRONTEND=noninteractive

# Default work directory gpp Gameplay Programming
WORKDIR /gpp

# Install apt-utils to prevent warnings and qemu for ARM emulation
RUN apt-get update \
	&& apt-get install -y apt-utils qemu-user \
	&& rm -rf /var/lib/apt/lists/*

# Add architectures and install dependencies
RUN dpkg --add-architecture arm64 \
	&& dpkg --add-architecture i386 \
	&& apt-get update \
	&& apt-get install -y \
	# Common packages
	build-essential \
	git \
	pkg-config \
	wget \
	tar \
	python3 \
	# Ping Tests
	iputils-ping \
	# Debugging and memory profiling
	valgrind \
	# AArch64 cross compile tools
	gcc-aarch64-linux-gnu \
	g++-aarch64-linux-gnu \
	gdb-multiarch \
	# x86_64 host packages Linux builds
	libegl1-mesa-dev \
	libgles2-mesa-dev \
	libdrm-dev \
	libgbm-dev \
	libx11-dev \
	libxrandr-dev \
	libxinerama-dev \
	libxcursor-dev \
	libxi-dev \
	# aarch64 target packages r36s builds
	libegl1-mesa-dev:arm64 \
	libgles2-mesa-dev:arm64 \
	libdrm-dev:arm64 \
	libgbm-dev:arm64 \
	libegl1:arm64 \
	libgles2:arm64 \
	libdrm2:arm64 \
	libgbm1:arm64 \
	libx11-6:arm64 \
	# Basic editor consider emacs
	nano \
	# MinGW-w64 for Windows builds
	mingw-w64 \
	# Wine installation to run Raylib Windows builds
	wine32:i386 \
	wine64 \
	# OpenGL utilities
	mesa-utils \
	libgl1-mesa-dri \
	x11-apps \
	# X11 libraries
	libxext6 \
	libxrender1 \
	libxfixes3 \
	libxxf86vm1 \
	libglx-mesa0 \
	libgl1 \
	# i386 libraries for Wine
	libgl1:i386 \
	libglx-mesa0:i386 \
 	libx11-6:i386 \
	libxext6:i386 \
	libxrender1:i386 \
	libxfixes3:i386 \
	libxi6:i386 \
	libxrandr2:i386 \
	libgl1-mesa-dri:i386 \
	# Generate compilation database 
	bear \
	# Clean up apt cache
	&& rm -rf /var/lib/apt/lists/*

# Generate SSH keypair
# This will allow SSH communication with the r36s without password prompts
RUN mkdir -p /root/.ssh && \
	ssh-keygen -t rsa -b 4096 -C "r36s-docker" -f /root/.ssh/id_r36s -N "" && \
	chmod 600 /root/.ssh/id_r36s && \
	echo "Host *\n  StrictHostKeyChecking no\n  UserKnownHostsFile=/dev/null" > /root/.ssh/config

# Install CMake for building raylib
ARG CMAKE_VERSION=3.27.9
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz \
	&& tar -xzf cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz \
	&& mv cmake-${CMAKE_VERSION}-linux-x86_64 /opt/cmake \
	&& ln -s /opt/cmake/bin/* /usr/local/bin/ \
	&& rm cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz

# Clone and install Emscripten SDK
ARG EMSDK_VERSION=4.0.21
ENV EMSDK=/opt/emsdk
RUN git clone https://github.com/emscripten-core/emsdk.git ${EMSDK} \
	&& cd ${EMSDK} \
	&& ./emsdk install ${EMSDK_VERSION} \
	&& ./emsdk activate ${EMSDK_VERSION}

# Install psmisc to restart webserver if in use
RUN apt-get update && apt-get install -y --no-install-recommends psmisc \
 && rm -rf /var/lib/apt/lists/*

# Clone raylib
ARG RAYLIB_VERSION=5.5
ENV RAYLIB_DIR=/opt/raylib
RUN git clone --depth 1 --branch ${RAYLIB_VERSION} https://github.com/raysan5/raylib.git ${RAYLIB_DIR}

# Build raylib for AArch64 GLES2 on r36s
ENV RAYLIB_R36S=${RAYLIB_DIR}/build/r36s
RUN mkdir -p ${RAYLIB_R36S} \
	&& cd ${RAYLIB_R36S} \
	&& cmake ${RAYLIB_DIR} \
		-DCMAKE_SYSTEM_NAME=Linux \
		-DCMAKE_SYSTEM_PROCESSOR=aarch64 \
		-DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
		-DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++ \
		-DPLATFORM=DRM \
		-DGRAPHICS=GRAPHICS_API_OPENGL_ES2 \
		-DBUILD_EXAMPLES=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_C_FLAGS="-std=gnu11 -Wno-stringop-overflow -I/usr/include/libdrm" \
		&& cmake --build . -j$(nproc) --verbose

# Build raylib for GLES2 on Linux
ENV RAYLIB_LINUX=${RAYLIB_DIR}/build/linux
RUN mkdir -p ${RAYLIB_LINUX} \
	&& cd ${RAYLIB_LINUX} \
	&& cmake ${RAYLIB_DIR} \
		-DPLATFORM=Desktop \
		-DGRAPHICS=GRAPHICS_API_OPENGL_ES2 \
		-DBUILD_EXAMPLES=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_FLAGS="-std=gnu11 -Wno-stringop-overflow" \
		&& cmake --build . -j$(nproc)

# Build raylib library for Emscripten SDK
ENV RAYLIB_WEB=${RAYLIB_DIR}/build/web
RUN mkdir -p ${RAYLIB_WEB} \
	&& cd ${EMSDK} \
	&& . ./emsdk_env.sh \
	&& cd ${RAYLIB_WEB} \
	&& emcmake cmake ../.. \
		-DPLATFORM=Web \
		-DBUILD_EXAMPLES=OFF \
		-DBUILD_GLFW=OFF \
		-DUSE_WAYLAND=OFF \
		-DUSE_X11=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		&& cmake --build . -j$(nproc)

# Build raylib for Windows (MinGW-w64 cross compile)
ENV RAYLIB_WINDOWS=${RAYLIB_DIR}/build/windows
RUN mkdir -p ${RAYLIB_WINDOWS} \
	&& cd ${RAYLIB_WINDOWS} \
	&& cmake ${RAYLIB_DIR} \
		-DCMAKE_SYSTEM_NAME=Windows \
		-DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
		-DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
		-DPLATFORM=Desktop \
		-DGRAPHICS=GRAPHICS_API_OPENGL_33 \
		-DBUILD_EXAMPLES=OFF \
		-DBUILD_SHARED_LIBS=OFF \
		-DCMAKE_BUILD_TYPE=Release \
		-DCMAKE_C_FLAGS="-std=gnu11 -Wno-stringop-overflow" \
		&& cmake --build . -j$(nproc)

# Add raylib include/lib paths to environment
ENV RAYLIB_INCLUDE=${RAYLIB_DIR}/src

# Environment variable to switch between AArch64 and x64 builds
ENV RAYLIB_LIB_AARCH64=${RAYLIB_R36S}/raylib
ENV RAYLIB_LIB_X64=${RAYLIB_LINUX}/raylib
ENV RAYLIB_LIB_WEB=${RAYLIB_WEB}/raylib
ENV RAYLIB_LIB_WIN64=${RAYLIB_WINDOWS}/raylib
