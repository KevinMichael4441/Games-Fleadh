#=================================================================
# Silence command echoing
#=================================================================
.SILENT:

#------------------------------------------------------------------
# Resource directory
#------------------------------------------------------------------
RESOURCE_DIR			:= ./resources
CONFIG_DIR				:= ./config
WEB_DIR					:= ./web
RULES_DIR				:= ./rules
ACHIEVEMENTS_DATA		:= ./data

#=================================================================
# Configuration
#=================================================================
# Messages and Printing Messages include
include $(RESOURCE_DIR)/resources.mk
include $(RESOURCE_DIR)/messages.mk

#=================================================================
# R36S Deployment Configuration
#=================================================================
CONFIG_FILE := $(CONFIG_DIR)/config.mk
CONFIG_TEMPLATE := $(CONFIG_DIR)/config.mk.template

# If config.mk does'nt exist, create it from config.mk.template
$(CONFIG_FILE):
	$(call WARNING_MSG,"config.mk not found. Creating from template")
	@cp $(CONFIG_TEMPLATE) $(CONFIG_FILE)

-include $(CONFIG_FILE)

#-----------------------------------------------------------------
# Strip config values (tabs/spaces)
#-----------------------------------------------------------------
GAME_NAME				:= $(strip $(GAME_NAME))
GAME_DESCRIPTION		:= $(strip $(GAME_DESCRIPTION))
GAME_VERSION			:= $(strip $(GAME_VERSION))
GAME_PATH				:= $(strip $(GAME_PATH))
GAME_TARGET				:= $(strip $(GAME_TARGET))

R36S_USER				:= $(strip $(R36S_USER))
R36S_HOST				:= $(strip $(R36S_HOST))
R36S_PATH				:= $(strip $(R36S_PATH))

R36S_GDB_PORT			:= $(strip $(R36S_GDB_PORT))
R36S_CONTROL_SOCKET		:= $(strip $(R36S_CONTROL_SOCKET))
PING_TIMEOUT			:= $(strip $(PING_TIMEOUT))

R36S_SSH_KEY_PRIV		:= $(strip $(R36S_SSH_KEY_PRIV))
R36S_SSH_KEY_PUB		:= $(strip $(R36S_SSH_KEY_PUB))

CP_SRC_FILES			:= $(strip $(CP_SRC_FILES))
CP_DESTINATION			:= $(strip $(CP_DESTINATION))

#-----------------------------------------------------------------
# For Shell Scripts
#-----------------------------------------------------------------
export GAME_NAME
export GAME_DESCRIPTION
export GAME_VERSION
export GAME_PATH
export GAME_TARGET

export R36S_USER
export R36S_HOST
export R36S_PATH

export R36S_GDB_PORT
export R36S_CONTROL_SOCKET
export PING_TIMEOUT

export R36S_SSH_KEY_PRIV
export R36S_SSH_KEY_PUB

export CP_SRC_FILES
export CP_DESTINATION

#=================================================================
# Default Build Target
#=================================================================
.DEFAULT_GOAL			:= all

#=================================================================
# Default Build Types
#=================================================================
RELEASE_BUILD			:= release
DEBUG_BUILD				:= debug

#=================================================================
# Target Platforms
#=================================================================
R36S_TARGET				:= r36s
LINUX_TARGET			:= linux
WEB_TARGET				:= web
WINDOWS_TARGET			:= windows

#-----------------------------------------------------------------
# Platform-specific file extensions
#-----------------------------------------------------------------
R36S_EXT				:=
LINUX_EXT				:=
WEB_EXT					:= .html
WINDOWS_EXT				:= .exe

#=================================================================
# Build Variables (Arguments)
#=================================================================

#-----------------------------------------------------------------
# Default to web target and debug build if not specified
#-----------------------------------------------------------------
PLATFORM 				?= $(WEB_TARGET)
CONFIG   				?= $(DEBUG_BUILD)

PLATFORM				:= $(strip $(PLATFORM))
CONFIG					:= $(strip $(CONFIG))

PLATFORM 				:= $(shell printf '%s' '$(PLATFORM)' | tr '[:upper:]' '[:lower:]')
CONFIG   				:= $(shell printf '%s' '$(CONFIG)' | tr '[:upper:]' '[:lower:]')

#=================================================================
# GPP Build Variables
#=================================================================
BASE_NAME				:= $(GAME_TARGET)
BUILD_ROOT				:= build
BUILD_DIR				:= $(BUILD_ROOT)/$(PLATFORM)_$(CONFIG)
TARGET_NAME				:= $(BASE_NAME)_$(PLATFORM)
TARGET					:= $(BUILD_DIR)/$(TARGET_NAME)

#=================================================================
# Platform build rules
#=================================================================
include $(RULES_DIR)/platforms.mk
include $(RULES_DIR)/qemu.mk
include $(RULES_DIR)/r36s.mk
include $(RULES_DIR)/linux.mk
include $(RULES_DIR)/web.mk
include $(RULES_DIR)/windows.mk

#=================================================================
# Raylib paths are set in Dockerfile see ENV section
#=================================================================
CFLAGS					:= -I$(XXHASH_INCLUDE) \
							-I$(CUTE_HEADERS_INCLUDE) \
							-I$(SPINE_INCLUDE) \
							-I$(RAYLIB_INCLUDE) \
							-Iinclude \
							-std=gnu11

#------------------------------------------------------------------
# Define Game Specific metadata for conditional compilation
# NOTE: Wrapped in single quotes to preserve spaces and 
# special characters for C preprocessor
#------------------------------------------------------------------
CFLAGS					+= '-DGAME_NAME=$(GAME_NAME)'
CFLAGS 					+= '-DGAME_DESCRIPTION=$(GAME_DESCRIPTION)'
CFLAGS 					+= '-DGAME_VERSION=$(GAME_VERSION)'

#=================================================================
# Source files (recursive if there are sub directories)
#=================================================================
# Source files - wildcard includes all .c files in src/
C_SRC					:= $(shell find src -name '*.c' 2>/dev/null)

#=================================================================
# Object files
#=================================================================
C_OBJ					:= $(patsubst src/%.c,$(BUILD_DIR)/obj/%.o,$(C_SRC))
OBJ						:= $(C_OBJ)

#=================================================================
# Header file updates (dependency tracking)
#=================================================================
CFLAGS					+= -MMD -MP
DEPS 					:= $(OBJ:.o=.d)
-include $(DEPS)

#=================================================================
# Emscripten paths
#=================================================================
EMSDK					:= /opt/emsdk
EMSCRIPTEN				:= $(EMSDK)/upstream/emscripten

#=================================================================
# Assets directory
#=================================================================
ASSETS_DIR				:= ./assets

#=================================================================
# Platform configuration
#=================================================================
LDFLAGS 				:=
LIBDIRS 				:=
LDLIBS  				:=

#-----------------------------------------------------------------
# R36S aarch64 target
#-----------------------------------------------------------------
ifeq ($(PLATFORM),$(R36S_TARGET))
	CC 					:= aarch64-linux-gnu-gcc

	# Define PLATFORM_R36S for conditional compilation
	CFLAGS				+= -DPLATFORM_R36S

	# RK3326 (Cortex-A35) specific optimizations
	# CRITICAL: These flags provide approximately 50 percent CPU reduction
	# - mcpu=cortex-a35: Optimize for Cortex-A35 specifically
	# - ffast-math: Aggressive math optimisations
	# - ftree-vectorize: Auto-vectorize loops with NEON
	# - funroll-loops: Unroll hot loops
	CFLAGS += -mcpu=cortex-a35 -mtune=cortex-a35
	CFLAGS += -ffast-math -ftree-vectorize -funroll-loops
	CFLAGS += -fomit-frame-pointer

	# Link-time optimization for release builds only
	# Provides additional 5-10 percent speedup but increases compile time
	ifeq ($(CONFIG),$(RELEASE_BUILD))
		CFLAGS			+= -flto
		LDFLAGS			+= -flto
	endif

	# R36S binary extension (blank)
	TARGET_EXT			:= $(R36S_EXT)

	# RAYLIB_LIB_AARCH64 is set in Dockerfile
	LIBDIRS 			+= -Laarch64-linux-gnu-gcc/lib \
						-L$(XXHASH_LIB_AARCH64) \
						-L$(SPINE_LIB_AARCH64) \
						-L$(RAYLIB_LIB_AARCH64)

	LDLIBS				+= -lraylib \
						-lspine-c\
						-lxxhash \
						-lm \
						-ldl \
						-lpthread \
						-lrt \
						-lEGL \
						-lGLESv2 \
						-ldrm \
						-lgbm

#-----------------------------------------------------------------
# Linux x64 target
#-----------------------------------------------------------------
else ifeq ($(PLATFORM),$(LINUX_TARGET))
	CC 					:= gcc

	# Define PLATFORM_LINUX for conditional compilation
	CFLAGS				+= -DPLATFORM_LINUX

	# Linux binary extension (blank)
	TARGET_EXT			:= $(LINUX_EXT)

	# RAYLIB_LIB_X64 is set in Dockerfile
	LIBDIRS 			+= -L$(XXHASH_LIB_X64) \
						-L$(SPINE_LIB_X64) \
						-L$(RAYLIB_LIB_X64)

	LDLIBS  			+= -lraylib \
						-lspine-c \
						-lxxhash \
						-lm \
						-ldl \
						-lpthread \
						-lrt \
						-lEGL \
						-lGLESv2 \
						-lX11

#------------------------------------------------------------------
# Web target using emscripten
#------------------------------------------------------------------
else ifeq ($(PLATFORM),$(WEB_TARGET))
	CC					:= $(EMSCRIPTEN)/emcc

	# Define DPLATFORM_WEB for conditional compilation
	CFLAGS				+= -DPLATFORM_WEB

	# Web binary extension html (Emscripten generates an HTML file by default)
	TARGET_EXT			:= $(WEB_EXT)
	HTML_TEMPLATE		:= $(WEB_DIR)/template.html
	
	# RAYLIB_LIB_WEB is set in Dockerfile
	LIBDIRS 			+= -L$(XXHASH_LIB_WEB) \
						-L$(SPINE_LIB_WEB) \
						-L$(RAYLIB_LIB_WEB)

	LDLIBS  			+= -lraylib \
						-lspine-c \
						-lxxhash

	LDFLAGS				+= --shell-file $(HTML_TEMPLATE) \
						-s USE_GLFW=3 \
						-s ASYNCIFY \
						-s INITIAL_MEMORY=134217728 \
						-s ALLOW_MEMORY_GROWTH=1 \
						-s 'EXPORTED_RUNTIME_METHODS=["HEAPF32","HEAPU8","HEAP32"]' \
						--preload-file $(ASSETS_DIR)@/assets

#------------------------------------------------------------------
# Windows x64 target (MinGW cross-compile)
#------------------------------------------------------------------
else ifeq ($(PLATFORM),$(WINDOWS_TARGET))
	CC					:= x86_64-w64-mingw32-gcc

	# Define PLATFORM_WINDOWS for conditional compilation
	CFLAGS				+= -DPLATFORM_WINDOWS

	# Windows binary extension .exe
	TARGET_EXT			:= $(WINDOWS_EXT)

	# RAYLIB_LIB_WIN64 is set in Dockerfile
	LIBDIRS 			+= -L$(XXHASH_LIB_WIN64) \
						-L$(RAYLIB_LIB_WIN64) \
						-L$(SPINE_LIB_WIN64)

	LDLIBS  			+= -lraylib \
						-lspine-c \
						-lxxhash \
						-lopengl32 \
						-lgdi32 \
						-lwinmm \
						-lkernel32 \
						-luser32 \
						-lshell32

	LDFLAGS				+= -static-libgcc
else
  $(call ERROR_MSG,$(MSG_HELP_PLATFORM))
  $(error Invalid PLATFORM '$(PLATFORM)'. Expected one of: $(R36S_TARGET) $(LINUX_TARGET) $(WEB_TARGET) $(WINDOWS_TARGET))
endif

#=================================================================
# Build configuration (optimisation 
# setting and debug flags)
#=================================================================
ifeq ($(CONFIG),$(DEBUG_BUILD))
	CFLAGS				+= -O0 -g -DDEBUG
else ifeq ($(CONFIG),$(RELEASE_BUILD))
	CFLAGS				+= -O3 -DNDEBUG
else
  # or a config-specific message
  $(call ERROR_MSG,$(MSG_HELP_CONFIG))
  $(error Invalid CONFIG '$(CONFIG)'. Expected one of: $(DEBUG_BUILD) $(RELEASE_BUILD))
endif

#=================================================================
# all target
#=================================================================
.PHONY: all \
		__validate_platform_value \
		__validate_config_value

all: 	__validate_platform_value \
		__validate_config_value \
		$(TARGET)

#=================================================================
# Bear for IntelliSense DB
#=================================================================
.PHONY: compile_only

compile_only: $(OBJ)

#=================================================================
# Compile C source files
#=================================================================
$(BUILD_DIR)/obj/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

#=================================================================
# Build target and create symlink to target build
#=================================================================
$(TARGET): $(OBJ)
	$(call SUCCESS_MSG,Build Started: $(TARGET))
	$(call INFO_MSG,$(MSG_BUILD_START))
	$(call INFO_MSG,Building: \"$(TARGET)$(TARGET_EXT)\")
	$(call INFO_MSG,PLATFORM=$(PLATFORM) CONFIG=$(CONFIG))
	mkdir -p $(BUILD_DIR)
	$(CC) $(OBJ) -o $(TARGET)$(TARGET_EXT) $(LDFLAGS) $(LIBDIRS) $(LDLIBS)
	ln -sf "$(notdir $@$(TARGET_EXT))" "$(BUILD_DIR)/target_$(PLATFORM)$(TARGET_EXT)"
ifeq ($(CONFIG),$(RELEASE_BUILD))
ifneq ($(PLATFORM),$(WEB_TARGET))
	$(call INFO_MSG,$(MSG_COPY_ASSETS) $(ASSETS_DIR) to $(BUILD_DIR)/)
	cp -r $(ASSETS_DIR) $(BUILD_DIR)/
endif
endif
	$(call SUCCESS_MSG,Build complete: $(TARGET))
	$(call SUCCESS_MSG,$(MSG_BUILD_END))


#=================================================================
# VSCode configuration values (settings.json)
#=================================================================
.PHONY: print-game-name \
		print-game-description \
		print-game-version \
		print-game-target \
		print-r36s-user \
		print-r36s-host \
		print-r36s-path \
		print-r36s-gdb-port \
		print-ping-timeout \
		print-r36s-control-socket \
		print-r36s-ssh-key-priv \
		print-r36s-ssh-key-pub \
		print-r36s-config

#-----------------------------------------------------------------
# Print values (for VSCode ${command:} usage)
#-----------------------------------------------------------------
print-game-name:
	@echo $(GAME_NAME)
print-game-description:
	@echo $(GAME_DESCRIPTION)
print-game-version:
	@echo $(GAME_VERSION)
print-game-target:
	@echo $(GAME_TARGET)
print-r36s-user:
	@echo $(R36S_USER)
print-r36s-host:
	@echo $(R36S_HOST)
print-r36s-path:
	@echo $(R36S_PATH)
print-r36s-gdb-port:
	@echo $(R36S_GDB_PORT)
print-ping-timeout:
	@echo $(PING_TIMEOUT)
print-r36s-control-socket:
	@echo $(R36S_CONTROL_SOCKET)
print-r36s-ssh-key-priv:
	@echo $(R36S_SSH_KEY_PRIV)
print-r36s-ssh-key-pub:
	@echo $(R36S_SSH_KEY_PUB)
	
#-----------------------------------------------------------------
# Print all config settings (for debugging)
#-----------------------------------------------------------------
print-r36s-config:
	@echo "R36S Configuration:"
	@echo "\tGAME_NAME: $(GAME_NAME) v$(GAME_VERSION)"
	@echo "\tGAME_DESCRIPTION: $(GAME_DESCRIPTION)"
	@echo "\tR36S_USER: $(R36S_USER)"
	@echo "\tR36S_HOST: $(R36S_HOST)"
	@echo "\tR36S_PATH: $(R36S_PATH)"
	@echo "\tR36S_GDB_PORT: $(R36S_GDB_PORT)"
	@echo "\tPING_TIMEOUT: $(PING_TIMEOUT)"
	@echo "\tR36S_CONTROL_SOCKET: $(R36S_CONTROL_SOCKET)"
	@echo "\tR36S_SSH_KEY_PRIV: $(R36S_SSH_KEY_PRIV)"
	@echo "\tR36S_SSH_KEY_PUB: $(R36S_SSH_KEY_PUB)"

#-----------------------------------------------------------------
# Sync VSCode settings from config/config.mk
#-----------------------------------------------------------------
sync-vscode-settings:
	$(call INFO_MSG,Syncing VSCode settings from config/config.mk)
	@bash scripts/sync_vscode_settings.sh
	@git update-index --skip-worktree .vscode/settings.json
	$(call SUCCESS_MSG,VSCode settings updated)

#=================================================================
# SSH Connection Management
#=================================================================
.PHONY: r36s_ssh_connection_up \
		r36s_ssh_connection_down \
		r36s_ssh_connection_status \
		r36s-ssh-explicit-key-enable \
		r36s-ssh-explicit-key-disable

r36s_ssh_connection_up:
	@R36S_CONTROL_SOCKET=$(R36S_CONTROL_SOCKET) \
	R36S_USER=$(R36S_USER) \
	R36S_HOST=$(R36S_HOST) \
	bash scripts/r36s_ssh_connection_up.sh

r36s_ssh_connection_down:
	@R36S_CONTROL_SOCKET=$(R36S_CONTROL_SOCKET) \
	R36S_USER=$(R36S_USER) \
	R36S_HOST=$(R36S_HOST) \
	bash scripts/r36s_ssh_connection_down.sh

r36s_ssh_connection_status:
	@R36S_CONTROL_SOCKET=$(R36S_CONTROL_SOCKET) \
	R36S_USER=$(R36S_USER) \
	R36S_HOST=$(R36S_HOST) \
	bash scripts/r36s_ssh_connection_status.sh

r36s-ssh-explicit-key-enable: r36s_ssh_connection_up
	./scripts/r36s_ssh_enable_key.sh

r36s-ssh-explicit-key-disable: r36s_ssh_connection_up
	./scripts/r36s_ssh_disable_key.sh

#=================================================================
# Platform/Config presets for convenience
#=================================================================
.PHONY: debug_r36s \
		debug_linux \
		debug_web \
		release_r36s \
		release_linux \
		release_web \
		debug_windows \
		release_windows \
		release_on_r36s_with_ssh_default \
		debug_on_r36s_with_ssh_default \
		release_on_r36s_with_ssh_explicit_key \
		debug_on_r36s_with_ssh_explicit_key

debug_r36s:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_r36s

debug_linux:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	PLATFORM=$(LINUX_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_linux

debug_web:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	PLATFORM=$(WEB_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_web

debug_windows:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	PLATFORM=$(WINDOWS_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_windows

release_r36s:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_r36s

release_on_r36s_with_ssh_default: r36s_ssh_connection_up
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_on_r36s_with_ssh_default

debug_on_r36s_with_ssh_default: r36s_ssh_connection_up
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_on_r36s_with_ssh_default

release_on_r36s_with_ssh_explicit_key:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_on_r36s_with_ssh_explicit_key

debug_on_r36s_with_ssh_explicit_key:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(R36S_TARGET) \
	CONFIG=$(DEBUG_BUILD) \
	__debug_on_r36s_with_ssh_explicit_key

release_linux:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(LINUX_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_linux

release_web:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(WEB_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_web

release_windows:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) \
	--no-print-directory \
	PLATFORM=$(WINDOWS_TARGET) \
	CONFIG=$(RELEASE_BUILD) \
	__release_windows

#=================================================================
# Rebuild Database
# Adding new '.c' files requires regenerating 'compile_commands.json'
# with 'bear'. This keeps IntelliSense in sync
# NOTE: Only run when a new '.c' file is added to build
#=================================================================
.PHONY: rebuild_component_db

rebuild_component_db:
	rm -f compile_commands.json
	TMPDIR=/tmp bear -- \
	  $(MAKE) -B -f $(firstword $(MAKEFILE_LIST)) \
	  PLATFORM=linux \
	  CONFIG=debug \
	  compile_only \
	  -j1

#=================================================================
# Clean targets
#=================================================================
.PHONY: clean clean_all clean_achievements

#-----------------------------------------------------------------
# Clean achievements
#-----------------------------------------------------------------
clean_achievements:
	$(call INFO_MSG,$(MSG_CLEAN_START))
	rm -rf $(ACHIEVEMENTS_DATA)/*
	$(call SUCCESS_MSG,$(MSG_CLEAN_END))

#-----------------------------------------------------------------
# Clean target (platform-specific)
#-----------------------------------------------------------------
clean: clean_achievements
	$(call INFO_MSG,$(MSG_CLEAN_START))
	rm -rf $(BUILD_DIR)
	$(call SUCCESS_MSG,Removed target: $(BUILD_DIR))
	$(call SUCCESS_MSG,$(MSG_CLEAN_END))

#-----------------------------------------------------------------
# Clean all targets (no validation needed)
#-----------------------------------------------------------------
clean_all: clean_achievements
	$(call INFO_MSG,$(MSG_CLEAN_START))
	rm -rf ./$(BUILD_ROOT)/
	$(call SUCCESS_MSG,Cleaned all build targets ./$(BUILD_ROOT)/)
	$(call SUCCESS_MSG,$(MSG_CLEAN_END))

#=================================================================
# Test all build targets
#=================================================================
.PHONY: test_all

test_all: clean_all
	$(call INFO_MSG,Running full build matrix test...)
	@bash scripts/test_all_builds.sh
	$(call SUCCESS_MSG,All build targets completed successfully)

#=================================================================
# Makefile Help
#=================================================================
.PHONY: help

help:
	$(call INFO_MSG_BOX,$(MSG_HELP_HEADER))
	$(call INFO_MSG,$(MSG_HELP_DEFAULT))
	$(call INFO_MSG,$(MSG_HELP_PLATFORM))
	$(call INFO_MSG,$(MSG_HELP_DEBUG_R36S))
	$(call INFO_MSG,$(MSG_HELP_DEBUG_LINUX))
	$(call INFO_MSG,$(MSG_HELP_DEBUG_WEB))
	$(call INFO_MSG,$(MSG_HELP_DEBUG_WINDOWS))
	$(call INFO_MSG,$(MSG_HELP_RELEASE_R36S))
	$(call INFO_MSG,$(MSG_HELP_RELEASE_LINUX))
	$(call INFO_MSG,$(MSG_HELP_RELEASE_WEB))
	$(call INFO_MSG,$(MSG_HELP_RELEASE_WINDOWS))
	$(call INFO_MSG,$(MSG_HELP_CLEAN))
	$(call INFO_MSG,$(MSG_HELP_CLEAN_ALL))
	$(call INFO_MSG,$(MSG_HELP_HELP))