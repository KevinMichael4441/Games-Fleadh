# ----------------------------------------
# Silence command echoing
# ----------------------------------------
.SILENT:

# ----------------------------------------
# Makefile with project targets
# (run inside Docker container)
# ----------------------------------------
MAKEFILE		:= Makefile.mk

# R36S Deployment Configuration
RESOURCE_DIR	:= ./resources
CONFIG_DIR		:= ./config

# Local with GPU vs Remote Server without GPU
GPU_AVAILABLE	:=
ifeq ($(wildcard /dev/dri),/dev/dri)
GPU_AVAILABLE	:= -f docker-compose-gpu.yml
endif

include $(CONFIG_DIR)/config.mk
include $(RESOURCE_DIR)/resources.mk
include $(RESOURCE_DIR)/messages.mk

CP_SRC_FILES 	:= $(strip $(CP_SRC_FILES))
CP_DESTINATION	:= $(strip $(CP_DESTINATION))

# ----------------------------------------
# Docker container base name
# ----------------------------------------
CONTAINER		:= gpp

# ----------------------------------------
# Phony targets
# ----------------------------------------
.PHONY: up run shell down rebuild help all\
        debug_web release_web \
		debug_linux release_linux \
		debug_r36s release_r36s \
		debug_windows release_windows \
		clean clean_all \
		xhost

# ----------------------------------------
# Default goal
# ----------------------------------------
.DEFAULT_GOAL 	:= all

# ----------------------------------------
# OS detection (host)
# ----------------------------------------
UNAME_S      	:= $(shell uname -s)
IS_WINDOWS   	:= $(if $(findstring MINGW,$(UNAME_S)),TRUE,FALSE)
IS_LINUX     	:= $(if $(findstring Linux,$(UNAME_S)),TRUE,FALSE)
IS_WSL       	:= $(shell grep -qi Microsoft /proc/version 2>/dev/null && echo TRUE || echo FALSE)
IS_MACOS     	:= $(if $(findstring Darwin,$(UNAME_S)),TRUE,FALSE)

# Export for Config Runs
export HOST_IS_WINDOWS := $(IS_WINDOWS)
export HOST_IS_LINUX   := $(IS_LINUX)
export HOST_IS_WSL     := $(IS_WSL)
export HOST_IS_MACOS   := $(IS_MACOS)

# ----------------------------------------
# Docker compose file selection
# ----------------------------------------
COMPOSE_BASE := docker-compose.yml

# Windows
ifeq ($(IS_WINDOWS),TRUE)
	COMPOSE_OS	:= docker-compose-windows.yml

# WSL Ubuntu Disto install works best in testing
else ifeq ($(IS_WSL),TRUE)
	COMPOSE_OS := docker-compose-wsl.yml

# Dawin MacOS
else ifeq ($(IS_MACOS),TRUE)
	COMPOSE_OS	:= docker-compose-macos.yml

# Linux Distro
else ifeq ($(IS_LINUX),TRUE)
	COMPOSE_OS := docker-compose-linux.yml

else
	$(error Unsupported OS)
endif

# Compose files
COMPOSE_FILES 	:= -f $(COMPOSE_BASE) -f $(COMPOSE_OS) $(GPU_AVAILABLE)

# Docker Compose
DC 				:= docker compose $(COMPOSE_FILES)

# ----------------------------------------
# Docker targets
# Start Docker containers
# X11 Docker container
# ----------------------------------------
# Allow X11 connections to Docker container
# so raylib, GLFW, Wine can create windows.
#
# If container runs as a non-root user
#   xhost +si:localuser:<username>
#
# Docs:
# https://docs.docker.com/config/containers/security/#x11-unix-socket

xhost:
	@command -v xhost >/dev/null 2>&1 && xhost +si:localuser:root >/dev/null 2>&1 || true

# Start Docker container in detached mode
up: xhost
	@$(DC) up -d --build

# ----------------------------------------
# Macro make commands inside Docker container
# ----------------------------------------
define DOCKER_MAKE
	@$(DC) exec -e HOST_IS_WINDOWS=$(HOST_IS_WINDOWS) \
	            -e HOST_IS_MACOS=$(HOST_IS_MACOS) \
	            -e HOST_IS_LINUX=$(HOST_IS_LINUX) \
	            -e HOST_IS_WSL=$(HOST_IS_WSL) \
	            $(CONTAINER) make -f $(MAKEFILE) $(MAKEOVERRIDES) $(1)
endef

# ----------------------------------------
# Default target
# ----------------------------------------
all: up
	@$(DC) exec -e HOST_IS_WINDOWS=$(HOST_IS_WINDOWS) \
	            -e HOST_IS_MACOS=$(HOST_IS_MACOS) \
	            -e HOST_IS_LINUX=$(HOST_IS_LINUX) \
	            -e HOST_IS_WSL=$(HOST_IS_WSL) \
	            $(CONTAINER) make -f $(MAKEFILE) $(MAKEOVERRIDES)

# ----------------------------------------
# Debug and Release targets
# ----------------------------------------
debug_web: up
	$(call DOCKER_MAKE,debug_web)

release_web: up
	$(call DOCKER_MAKE,release_web)

debug_linux: up
	$(call DOCKER_MAKE,debug_linux)

release_linux: up
	$(call DOCKER_MAKE,release_linux)

debug_r36s: up
	$(call DOCKER_MAKE,debug_r36s)

release_r36s: up
	$(call DOCKER_MAKE,release_r36s)

debug_on_r36s: up
	$(call DOCKER_MAKE,debug_on_r36s)

copy_r36s_sd: release_r36s
	$(call INFO_MSG_BOX, $(MSG_COPY_CONFIG))
	$(call INFO_MSG, $(MSG_COPY_TO_SD_START))
	$(call INFO_MSG, Copying files...\nSource: $(CP_SRC_FILES)\nDestination: $(CP_DESTINATION))
	sudo mkdir -p "$(CP_DESTINATION)"
	sudo cp -a $(CP_SRC_FILES) $(CP_DESTINATION)
	$(call SUCCESS_MSG, $(MSG_COPY_TO_SD_END))

# TODO : Handle Container Rebuild
setup_ssh: up
	$(call INFO_MSG_BOX, "Configuring SSH on R36S")
	$(call INFO_MSG_BOX, "Running shell script ./scripts/setup_ssh_r36s.sh")
	@$(DC) exec \
		-e R36S_USER=$(R36S_USER) \
		-e R36S_HOST=$(R36S_HOST) \
		$(CONTAINER) bash -lc 'chmod +x ./scripts/setup_ssh_r36s.sh && ./scripts/setup_ssh_r36s.sh "$${R36S_USER:-ark}" "$${R36S_HOST}"'

release_on_r36s: up
	$(call DOCKER_MAKE,release_on_r36s)

debug_windows: up
	$(call DOCKER_MAKE,debug_windows)

release_windows: up
	$(call DOCKER_MAKE,release_windows)
ifeq ($(HOST_IS_WINDOWS),TRUE)
	@./build/windows_release/gpp_windows.exe
endif

# ----------------------------------------
# Clean targets
# ----------------------------------------
clean: up
	$(call DOCKER_MAKE,clean)

clean_all: up
	$(call DOCKER_MAKE,clean_all)

# ----------------------------------------
# Help target
# ----------------------------------------
help:
	$(call DOCKER_MAKE,help)

# ----------------------------------------
# Open a shell inside Docker container
# if you want to run make now use
# "make -f Makefile.mk <target>"
# ----------------------------------------
shell: up
	@$(DC) exec -it $(CONTAINER) bash || docker compose exec -it $(CONTAINER) sh

# ----------------------------------------
# Stop Docker container
# ----------------------------------------
down:
	@$(DC) down

# ----------------------------------------
# Rebuild Docker container
# ----------------------------------------
rebuild:
	@$(DC) build --no-cache
