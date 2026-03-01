#=================================================================
# Silence command echoing
#=================================================================
.SILENT:

#=================================================================
# Makefile with project targets
# (run inside Docker container)
#=================================================================
MAKEFILE				:= Makefile.mk

# R36S Deployment Configuration
RESOURCE_DIR			:= ./resources
CONFIG_DIR				:= ./config

#=================================================================
# Resources
#=================================================================
include $(RESOURCE_DIR)/resources.mk
include $(RESOURCE_DIR)/messages.mk

#=================================================================
# Build Directory
#=================================================================
BUILD_DIR       		?= ./build

#=================================================================
# R36S Deployment Configuration
#=================================================================
CONFIG_USER_FILE 		:= $(CONFIG_DIR)/config.mk
CONFIG_TEMPLATE  		:= $(CONFIG_DIR)/config.mk.template

# Generated config in BUILD_DIR
CONFIG_GEN_DIR   		:= $(BUILD_DIR)/generated
CONFIG_GEN_FILE  		:= $(CONFIG_GEN_DIR)/config.mk

# If config exists use it otherwise use copy
ifeq ($(wildcard $(CONFIG_USER_FILE)),)
  CONFIG_FILE 			:= $(CONFIG_GEN_FILE)
else
  CONFIG_FILE 			:= $(CONFIG_USER_FILE)
endif

#-----------------------------------------------------------------
# Strip config values (tabs/spaces)
#-----------------------------------------------------------------
CP_SRC_FILES 			:= $(strip $(CP_SRC_FILES))
CP_DESTINATION			:= $(strip $(CP_DESTINATION))

R36S_USER				:= $(strip $(R36S_USER))
R36S_HOST				:= $(strip $(R36S_HOST))
R36S_PATH				:= $(strip $(R36S_PATH))

# For Shell Scripts
export R36S_USER
export R36S_HOST
export CONTROL_SOCKET
export R36S_PATH

#=================================================================
# Host user ID for container permissions
#=================================================================
export UID				:= $(shell id -u)
export GID				:= $(shell id -g)

#=================================================================
# Docker container base name
#=================================================================
CONTAINER				:= gpp

#=================================================================
# Auto Detect Environment
# Linux + Nvidia GPU + Dev Container
# Linux + AMD GPU / iGPU + Dev Container
# Windows 11 + Nvidia GPU + Dev Container
# Windows 11 + AMD GPU / iGPU + Dev Container
# Headless (no GPU, build only e.g CI server)
# Override with: make ENV=linux-amd <target>
#=================================================================

# WSL Detect
IS_WSL					:= $(shell grep -qi Microsoft /proc/version 2>/dev/null && echo TRUE || echo FALSE)

# GPU Detect
HAS_NVIDIA				:= $(shell test -e /dev/nvidia0 && echo TRUE || echo FALSE)
HAS_AMD					:= $(shell grep -rq "0x1002" /sys/class/drm/*/device/vendor 2>/dev/null && echo TRUE || echo FALSE)
HAS_DRI					:= $(shell test -d /dev/dri && echo TRUE || echo FALSE)

# Build OS string
ifeq ($(IS_WSL),TRUE)
  _DETECTED_OS			:= wsl
else
  _DETECTED_OS			:= linux
endif

# Build GPU string (Intel iGPU same DRI path as AMD)
# No GPU detected = headless (CI server)
# Local with GPU vs Remote Server without GPU
ifeq ($(HAS_NVIDIA),TRUE)
  _DETECTED_GPU 		:= nvidia
else ifeq ($(HAS_AMD),TRUE)
  ifeq ($(HAS_DRI),TRUE)
    _DETECTED_GPU		:= amd
  else
    _DETECTED_GPU		:= headless
  endif
else ifeq ($(HAS_DRI),TRUE)
  _DETECTED_GPU			:= igpu
else
  _DETECTED_GPU			:= headless
endif

# Headless bypasses OS string (no display, no audio, build only)
ifeq ($(_DETECTED_GPU),headless)
  ENV					?= headless
else
  ENV					?= $(_DETECTED_OS)-$(_DETECTED_GPU)
endif

# Final ENV string
ENV						:= $(strip $(ENV))

# Map iGPU to AMD config (same DRI setup)
ifeq ($(findstring igpu,$(ENV)),igpu)
  _CONFIG_ENV			:= $(subst igpu,amd,$(ENV))
else
  _CONFIG_ENV			:= $(ENV)
endif

# Validate
VALID_ENVS := linux-nvidia linux-amd linux-igpu wsl-nvidia wsl-amd wsl-igpu headless
ifeq ($(filter $(ENV),$(VALID_ENVS)),)
  $(error Invalid ENV '$(ENV)'. Valid options: $(VALID_ENVS))
endif

$(info ENV: $(ENV)  [override with: make ENV=linux-nvidia|linux-amd|linux-igpu|wsl-nvidia|wsl-amd|wsl-igpu|headless <target>])

export HOST_IS_WSL		:= $(IS_WSL)

#=================================================================
# Windows detection (for running .exe after build)
#=================================================================
_UNAME_S				:= $(shell uname -s)
_HOST_IS_WINDOWS		:= $(if $(findstring MINGW,$(_UNAME_S)),TRUE,FALSE)

# Export for Config Runs
export HOST_IS_WINDOWS	:= $(_HOST_IS_WINDOWS)

#=================================================================
# Compose file configuration
# Single source of truth: base + per-environment override
# Override files live in .devcontainer/$(ENV)/
#=================================================================
COMPOSE_BASE			:= docker-compose.yml
COMPOSE_OVERRIDE		:= .devcontainer/$(_CONFIG_ENV)/docker-compose.override.yml
COMPOSE_FILES			:= -f $(COMPOSE_BASE) -f $(COMPOSE_OVERRIDE)

# Docker Compose
DC						:= docker compose $(COMPOSE_FILES)

#=================================================================
# Phony targets
#=================================================================
.PHONY: up run shell down rebuild help all \
        debug_web release_web \
        debug_linux release_linux \
        debug_r36s release_r36s \
        debug_windows release_windows \
        clean clean_all \
        xhost \
        clean-vscode \
		prune \
		fix-permissions \
		rebuild \
        env_linux_nvidia \
        env_linux_amd \
        env_wsl_nvidia \
        env_wsl_amd \
        env_headless \
        print_environment

#=================================================================
# Default goal
#=================================================================
.DEFAULT_GOAL 	:= all

#=================================================================
# Environment preset convenience targets
#=================================================================
env_linux_nvidia:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) ENV=linux-nvidia up

env_linux_amd:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) ENV=linux-amd up

env_wsl_nvidia:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) ENV=wsl-nvidia up

env_wsl_amd:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) ENV=wsl-amd up

env_headless:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) ENV=headless up

#=================================================================
# Docker targets
# Start Docker containers
# X11 Docker container
#=================================================================
# Allow X11 connections to Docker container
# so raylib, GLFW, Wine can create windows.
#
# If container runs as a non-root user
#   xhost +si:localuser:<username>
#
# Docs:
# https://docs.docker.com/config/containers/security/#x11-unix-socket
#
# NOTE: Also called via devcontainer.json initializeCommand
# so VSCode devcontainer start also runs this logic
#=================================================================
xhost:
	# Fix corrupted .Xauthority...Docker sometimes creates it as a directory
	@if [ -d "$(HOME)/.Xauthority" ]; then \
		echo "WARNING: ~/.Xauthority is a directory - fixing..."; \
		rm -rf "$(HOME)/.Xauthority"; \
	fi
	# Grant Docker access to X11 display
	@command -v xhost >/dev/null 2>&1 && xhost +local:docker >/dev/null 2>&1 || true
	@command -v xhost >/dev/null 2>&1 && xhost +si:localuser:root >/dev/null 2>&1 || true

# Start Docker container in detached mode
up: xhost
	@$(DC) up -d --build

#=================================================================
# Macro make commands inside Docker container
#=================================================================
define DOCKER_MAKE
	@$(DC) exec -e HOST_IS_WINDOWS=$(HOST_IS_WINDOWS) \
	            -e HOST_IS_WSL=$(IS_WSL) \
	            $(CONTAINER) make -f $(MAKEFILE) $(MAKEOVERRIDES) $(1)
endef

#=================================================================
# Default target
#=================================================================
all: print_environment up
	@$(DC) exec -e HOST_IS_WINDOWS=$(HOST_IS_WINDOWS) \
	            -e HOST_IS_WSL=$(IS_WSL) \
	            $(CONTAINER) make -f $(MAKEFILE) $(MAKEOVERRIDES)

#=================================================================
# Debug and Release targets
#=================================================================
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

release_on_r36s: up
	$(call DOCKER_MAKE,release_on_r36s)

debug_windows: up
	$(call DOCKER_MAKE,debug_windows)

release_windows: up
	$(call DOCKER_MAKE,release_windows)
# TODO: Handle Windows build exe execution more robustly
ifeq ($(HOST_IS_WINDOWS),TRUE)
	@./build/windows_release/gpp_windows.exe
endif

#=================================================================
# TODO : Handle Container Rebuild
#=================================================================
setup_ssh: up
	$(call INFO_MSG_BOX, "Configuring SSH on R36S")
	$(call INFO_MSG_BOX, "Running shell script ./scripts/setup_ssh_r36s.sh")
	@$(DC) exec \
		-e R36S_USER=$(R36S_USER) \
		-e R36S_HOST=$(R36S_HOST) \
		$(CONTAINER) bash -lc 'chmod +x ./scripts/setup_ssh_r36s.sh && ./scripts/setup_ssh_r36s.sh "$${R36S_USER:-ark}" "$${R36S_HOST}"'

#=================================================================
# Clean targets
#=================================================================
clean: up
	$(call DOCKER_MAKE,clean)

clean_all: up
	$(call DOCKER_MAKE,clean_all)

#=================================================================
# Help target
#=================================================================
help:
	$(call DOCKER_MAKE,help)

#=================================================================
# Open a shell inside Docker container
# if you want to run make now use
# "make -f Makefile.mk <target>"
#=================================================================
shell: up
	@$(DC) exec -it $(CONTAINER) bash || docker compose exec -it $(CONTAINER) sh

#=================================================================
# Stop Docker container
#=================================================================
down:
	@$(DC) down

#=================================================================
# Detected environment (debug)
#=================================================================
print_environment:
	@echo "Host OS Detection:"
	if [ "$(_HOST_IS_WINDOWS)" = "TRUE" ]; then \
		echo "\tDetected Windows Host"; \
	elif [ "$(IS_WSL)" = "TRUE" ]; then \
		echo "\tDetected WSL Host"; \
	else \
		echo "\tDetected Linux Host"; \
	fi

	@echo "GPU Detection:"
	if [ "$(HAS_NVIDIA)" = "TRUE" ]; then \
		echo "\tDetected NVIDIA GPU"; \
	elif [ "$(HAS_AMD)" = "TRUE" ]; then \
		echo "\tDetected AMD GPU"; \
	elif [ "$(HAS_DRI)" = "TRUE" ]; then \
		echo "\tDetected iGPU (Intel or other)"; \
	else \
    	echo "\tNo GPU detected (Headless)"; \
	fi

	@echo "Environment:"
	@echo "\tENV:              $(ENV)"
	@echo "\tCOMPOSE_BASE:     $(COMPOSE_BASE)"
	@echo "\tCOMPOSE_OVERRIDE: $(COMPOSE_OVERRIDE)"

#=================================================================
# Remove 'vscode' Dev Containers plugin volume(s)
#=================================================================
clean-vscode:
	@echo "Stopping container(s) using vscode volume"
	@docker ps -a --filter volume=vscode -q | xargs -r docker rm -f
	@docker ps -a --filter volume=vsc -q | xargs -r docker rm -f
	@echo "Removing vscode volumes"
	@docker volume ls --filter "name=vscode" -q | xargs -r docker volume rm
	@docker volume ls --filter "name=vsc" -q | xargs -r docker volume rm

#=================================================================
# Prune removes container, image, build cache as it fills up 
# over time with rebuilds. This reclaims disk space and ensures clean builds.
#=================================================================
prune: down clean-vscode
	@echo "Removing image: r36s-build"
	@docker rmi r36s-build 2>/dev/null || true
	@echo "Pruning unused images"
	@docker image prune -a -f
	@echo "Pruning build cache"
	@docker builder prune -f
	@echo "Docker disk usage:"
	@docker system df
	
#=================================================================
# Set permissions after Docker makes them root access
#=================================================================
fix-permissions:
	@echo "Fixing file permissions"
	@sudo chown -R $(UID):$(GID) .
	@echo "Done"

#=================================================================
# Rebuild Docker container from scratch
#=================================================================
rebuild: prune
	@$(DC) build --no-cache
	@$(MAKE) fix-permissions