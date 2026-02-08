# ----------------------------------------
# Silence command echoing
# ----------------------------------------
.SILENT:

# Resource directory
RESOURCE_DIR	:= ./resources
CONFIG_DIR		:= ./config
WEB_DIR			:= ./web
RULES_DIR		:= ./rules

# ----------------------------------------
# Configuration
# ----------------------------------------
# Messages and Printing Messages include
include $(RESOURCE_DIR)/resources.mk
include $(RESOURCE_DIR)/messages.mk

# R36S Deployment Configuration
include $(CONFIG_DIR)/config.mk

# ----------------------------------------
# Default Build Target
# ----------------------------------------
.DEFAULT_GOAL	:= all

# ----------------------------------------
# Default Build Types
# ----------------------------------------
RELEASE_BUILD	:= release
DEBUG_BUILD		:= debug

# ----------------------------------------
# Build Variables (Arguments)
# ----------------------------------------
PLATFORM_ARG	?= web
CONFIG_ARG		?= $(DEBUG_BUILD)

PLATFORM		= $(shell echo $(PLATFORM_ARG) | tr '[:upper:]' '[:lower:]')
CONFIG			= $(shell echo $(CONFIG_ARG) | tr '[:upper:]' '[:lower:]')

# GPP Build Variables
BASE_NAME		:= gpp
BUILD_DIR		:= build/$(PLATFORM)_$(CONFIG)
TARGET_NAME		:= $(BASE_NAME)_$(PLATFORM)
TARGET			:= $(BUILD_DIR)/$(TARGET_NAME)

# Target Platforms
R36S_TARGET		:= r36s
LINUX_TARGET	:= linux
WEB_TARGET		:= web
WINDOWS_TARGET	:= windows

# Platform build rules
include $(RULES_DIR)/platforms.mk
include $(RULES_DIR)/qemu.mk
include $(RULES_DIR)/r36s.mk
include $(RULES_DIR)/linux.mk
include $(RULES_DIR)/web.mk
include $(RULES_DIR)/windows.mk

# SSH and R36S Deployment Configuration
# R36S Deployment Configuration moved to config/config.mk

# Raylib paths are set in Dockerfile see ENV section
CFLAGS			:= -I$(RAYLIB_INCLUDE) -Iinclude -std=gnu11
SRC				:= $(wildcard src/*.c)

# Emscripten paths
EMSDK			:= /opt/emsdk
EMSCRIPTEN		:= $(EMSDK)/upstream/emscripten

# Assets directory
ASSETS_DIR		:= ./assets

# ----------------------------------------
# Platform configuration
# ----------------------------------------
# R36S aarch64 target
ifeq ($(PLATFORM),$(R36S_TARGET))
	CC 				= aarch64-linux-gnu-gcc

	# Define PLATFORM_R36S for conditional compilation
	CFLAGS			+= -DPLATFORM_R36S

	TARGET_EXT		= # intentionally blank can be used for e.g .rom .exe .html 

	# RAYLIB_LIB_AARCH64 is set in Dockerfile
	LDFLAGS 		= -Laarch64-linux-gnu-gcc/lib \
					-L$(RAYLIB_LIB_AARCH64) \
					-lraylib \
					-lm \
					-ldl \
					-lpthread \
					-lrt \
					-lEGL \
					-lGLESv2 \
					-ldrm \
					-lgbm

# Linux x64 target
else ifeq ($(PLATFORM),$(LINUX_TARGET))
	CC 				= gcc

	# Define PLATFORM_LINUX for conditional compilation
	CFLAGS			+= -DPLATFORM_LINUX

	TARGET_EXT		= # intentionally blank can be used for e.g .rom .exe .html 

	# RAYLIB_LIB_X64 is set in Dockerfile
	LDFLAGS			= -L$(RAYLIB_LIB_X64) -lraylib \
					-lm \
					-ldl \
					-lpthread \
					-lrt \
					-lEGL \
					-lGLESv2 \
					-lX11

# Web target using emscripten
else ifeq ($(PLATFORM),$(WEB_TARGET))
	CC				= $(EMSCRIPTEN)/emcc

	# Define DPLATFORM_WEB for conditional compilation
	CFLAGS			+= -DPLATFORM_WEB

	TARGET_EXT		= .html
	HTML_TEMPLATE	= $(WEB_DIR)/template.html
	
	# RAYLIB_LIB_WEB is set in Dockerfile
	LDFLAGS			= -L$(RAYLIB_LIB_WEB) \
					-lraylib \
					--shell-file $(HTML_TEMPLATE) \
					-s USE_GLFW=3 \
					-s ASYNCIFY \
					-s INITIAL_MEMORY=134217728 \
					-s ALLOW_MEMORY_GROWTH=1 \
					--preload-file ./assets

# Windows x64 target (MinGW cross-compile)
else ifeq ($(PLATFORM),$(WINDOWS_TARGET))
	CC				= x86_64-w64-mingw32-gcc

	# Define PLATFORM_WINDOWS for conditional compilation
	CFLAGS			+= -DPLATFORM_WINDOWS

	TARGET_EXT		= .exe

	# RAYLIB_LIB_WIN64 is set in Dockerfile
	LDFLAGS			= -L$(RAYLIB_LIB_WIN64) -lraylib \
					-lopengl32 \
					-lgdi32 \
					-lwinmm \
					-lkernel32 \
					-luser32 \
					-lshell32
else
	$(call ERROR_MSG,$(MSG_HELP_PLATFORM))
	@exit 1
endif

# ----------------------------------------
# Build configuration (optimisation 
# setting and debug flags)
# ----------------------------------------
ifeq ($(CONFIG),$(DEBUG_BUILD))
	CFLAGS			+= -O0 -g -DDEBUG
else ifeq ($(CONFIG),$(RELEASE_BUILD))
	CFLAGS			+= -O3 -DNDEBUG
else
	$(call ERROR_MSG,$(MSG_HELP_PLATFORM))
	@exit 1
endif

# ----------------------------------------
# all target
# ----------------------------------------
.PHONY: all __validate_platform_value __validate_config_value

all: __validate_platform_value __validate_config_value $(TARGET)

# ----------------------------------------
# Build target
# ----------------------------------------
$(TARGET): $(SRC)
	$(call SUCCESS_MSG,Build Started: $(TARGET))
	$(call INFO_MSG,$(MSG_BUILD_START))
	$(call INFO_MSG,Building: \"$(TARGET)$(TARGET_EXT)\")
	$(call INFO_MSG,PLATFORM=$(PLATFORM) CONFIG=$(CONFIG))
	mkdir -p $(BUILD_DIR)
	$(CC) $(SRC) -o $(TARGET)$(TARGET_EXT) $(CFLAGS) $(LDFLAGS)
ifeq ($(CONFIG),$(RELEASE_BUILD))
ifneq ($(PLATFORM),$(WEB_TARGET))
	$(call INFO_MSG,$(MSG_COPY_ASSETS) $(ASSETS_DIR) to $(BUILD_DIR)/)
	cp -r $(ASSETS_DIR) $(BUILD_DIR)/
endif
endif
	$(call SUCCESS_MSG,Build complete: $(TARGET))
	$(call SUCCESS_MSG,$(MSG_BUILD_END))

# ----------------------------------------
# Platform/Config presets for convenience
# ----------------------------------------
.PHONY: debug_r36s debug_linux \
		debug_web release_r36s \
		release_linux \
		release_web

debug_r36s: 
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory PLATFORM=$(R36S_TARGET) CONFIG=$(DEBUG_BUILD) __debug_r36s || exit 1

debug_linux:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(LINUX_TARGET) CONFIG=$(DEBUG_BUILD) __debug_linux

debug_web:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(WEB_TARGET) CONFIG=$(DEBUG_BUILD) __debug_web

debug_windows:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(WINDOWS_TARGET) CONFIG=$(DEBUG_BUILD) __debug_windows

release_r36s:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory PLATFORM=$(R36S_TARGET) CONFIG=$(RELEASE_BUILD) __release_r36s || exit 1

release_on_r36s:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory PLATFORM=$(R36S_TARGET) CONFIG=$(RELEASE_BUILD) __release_on_r36s || exit 1

release_linux:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(LINUX_TARGET)  CONFIG=$(RELEASE_BUILD) __release_linux

release_web:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(WEB_TARGET) CONFIG=$(RELEASE_BUILD) __release_web

release_windows:
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) PLATFORM=$(WINDOWS_TARGET) CONFIG=$(RELEASE_BUILD) __release_windows

# ----------------------------------------
# Rebuild Database
# Adding new '.c' files requires regenerating 'compile_commands.json'
# with 'bear'. This keeps IntelliSense in sync
# NOTE: Only run when a new '.c' file is added to build
# ----------------------------------------
.PHONY: rebuild_component_db

rebuild_component_db:
	rm -f compile_commands.json
    TMPDIR=/tmp bear -- \
      $(MAKE) -B -f $(firstword $(MAKEFILE_LIST)) \
      PLATFORM=linux \
      CONFIG=debug \
      compile_only \
      -j1
# ----------------------------------------
# Clean targets
# ----------------------------------------
.PHONY: clean clean_all

# ----------------------------------------
# Clean target (platform-specific)
# ----------------------------------------
clean:
	$(call INFO_MSG,$(MSG_CLEAN_START))
	rm -rf $(BUILD_DIR)
	$(call SUCCESS_MSG,Removed target: $(BUILD_DIR))
	$(call SUCCESS_MSG,$(MSG_CLEAN_END))

# ----------------------------------------
# Clean all targets (no validation needed)
# ----------------------------------------
clean_all:
	$(call INFO_MSG,$(MSG_CLEAN_START))
	rm -rf ./build/
	$(call SUCCESS_MSG,Cleaned all build targets ./build)
	$(call SUCCESS_MSG,$(MSG_CLEAN_END))

# ----------------------------------------
# Makefile Help
# ----------------------------------------
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