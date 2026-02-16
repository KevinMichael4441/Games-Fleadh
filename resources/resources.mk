#=================================================================
# Build Messages
#=================================================================
MSG_BUILD_START			:= Game Build Started...
MSG_BUILD_END 			:= Game Build Complete.

MSG_CLEAN_START 		:= Cleaning up...
MSG_CLEAN_END 			:= Cleaned build target.

#=================================================================
# Web Messages
#=================================================================
MSG_WEB_CLICK_LINK_BELOW:= Click the link BELOW to open in your Web Browser	
MSG_WEB_CLICK_LINK_ABOVE:= Click the link ABOVE to open in your Web Browser

#=================================================================
# Error/Warning Messages
#=================================================================
MSG_INVALID_PLATFORM 	:= Invalid PLATFORM usage PLATFORM=r36s|linux|web
MSG_INVALID_CONFIG_DEBUG:= Invalid CONFIG usage CONFIG=debug
MSG_INVALID_CONFIG_RELEASE:= Invalid CONFIG usage CONFIG=release
MSG_USAGE_CONFIG 		:= Usage: make debug_r36s|debug_linux|debug_web|debug_windows 
MSG_USAGE_CONFIG_R36S 	:= Usage: make PLATFORM=r36s CONFIG=debug|release
MSG_USAGE_CONFIG_LINUX 	:= Usage: make PLATFORM=linux CONFIG=debug|release
MSG_USAGE_CONFIG_WEB 	:= Usage: make PLATFORM=web CONFIG=debug|release
MSG_USAGE_CONFIG_WINDOWS:= Usage: make PLATFORM=windows CONFIG=debug|release

#=================================================================
# QEMU is like a VM for arm
#=================================================================
MSG_QEMU_START 			:= Starting QEMU to test run r36s binary...
MSG_QEMU_END 			:= QEMU r36s binary test run complete...
MSG_QEMU_ERROR			:= QEMU stanity test failed

#=================================================================
# Windows Emulator
#=================================================================
MSG_WINE_START 			:= Starting Wine to test run Windows binary...
MSG_WINE_END 			:= Wine Windows binary test run complete...

#=================================================================
# SSH/Deployment Messages
#=================================================================
MSG_R36S_CONNECT_ERROR	:= Unable to connect to R36S\n\n \
							Ensure device is powered on and connected to WiFi.\n \
							Confirm IP address in 'config/config.mk' is correct.

MSG_R36S_SSH_EK_ERROR	:= Unable to connect to R36S\n\n \
							Ensure SSH is enabled, R36S is powered on and connected to WiFi.\n \
							Confirm IP address in 'config/config.mk' is correct.\n \
							Confirm SSH key is installed on R36S device.\n \
							Run: bash scripts/setup_ssh_r36s.sh

MSG_DEPLOY_START 		:= Deploying r36s binary to R36S device...
MSG_DEPLOY_END 			:= Deployment complete.
MSG_DEPLOY_STOP_GDB 	:= Stopping any running gdbserver on R36S...
MSG_DEPLOY_STOP_ES 		:= Stopping emulationstation on R36S...
MSG_DEPLOY_START_ES		:= Starting emulationstation on R36S...

MSG_COPY_CONFIG			:= Confirm directories in 'config/config.mk' are correct
MSG_COPY_TO_SD_START	:= Starting process copy files to SD Card...
MSG_COPY_TO_SD_END		:= Files copied to SD Card.

MSG_SSH_KEY_ERROR		:= SSH key not installed on R36S device.\nRun: bash scripts/setup_ssh_r36s.sh
MSG_SSH_KEY_SUCCESS		:= SSH key verified on R36S device.
MSG_SSH_CHECK			:= Checking SSH connection to R36S device...

MSG_DEPLOY_COPY 		:= Copying binary to R36S device...
MSG_DEPLOY_START_GDB 	:= Starting gdbserver on R36S...

MSG_COPY_ASSETS			:= Copying assets for release build...

#=================================================================
# Help Messages
#=================================================================
MSG_HELP_HEADER			:= Available 'make' targets \nEnsure Docker container is running\nRun 'make' inside Docker container

MSG_HELP_DEFAULT		:= make				: Build default (PLATFORM=web CONFIG=debug)\n
MSG_HELP_PLATFORM		:= make PLATFORM=<t> CONFIG=<c>\nmake PLATFORM=r36s|linux|web|windows CONFIG=debug|release\n


MSG_HELP_DEBUG_R36S		:= make debug_r36s			: Debug build for R36S
MSG_HELP_DEBUG_LINUX	:= make debug_linux			: Debug build for Linux
MSG_HELP_DEBUG_WEB		:= make debug_web			: Debug build for Web
MSG_HELP_DEBUG_WINDOWS	:= make debug_windows		: Debug build for Windows

MSG_HELP_RELEASE_R36S	:= make release_r36s			: Release build and deploy on R36S
MSG_HELP_RELEASE_LINUX	:= make release_linux		: Release build and run on Linux
MSG_HELP_RELEASE_WEB	:= make release_web			: Release build and run in Web Browser
MSG_HELP_RELEASE_WINDOWS:= make release_windows		: Release build and run on Windows

MSG_HELP_CLEAN			:= make clean			: Clean PLATFORM and CONFIG
MSG_HELP_CLEAN_ALL		:= make clean_all			: Clean all build targets
MSG_HELP_HELP			:= make help				: Show help message