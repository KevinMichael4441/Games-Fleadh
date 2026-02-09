#=================================================================
# Game Configuration
#
# Values may contain spaces.
# Use DOUBLE-QUOTED STRINGS for values that will be passed to C/C++
# via -D compiler defines (recommended).
#
# Examples:
#   GAME_NAME        = "My Game Name"
#   Approximately 20 to 30 characters for GAME_NAME, UI (title) has limited space.
#   GAME_DESCRIPTION = "Description of game"
#
# Do NOT escape spaces with backslashes.
# Quotes will be preserved and passed correctly to the compiler.
#
# If a value contains a literal double-quote character, escape it:
#   GAME_DESCRIPTION = "The game\"s story begins"
#=================================================================
GAME_NAME     		= "Game Name"
GAME_DESCRIPTION	= "Description of game"
GAME_VERSION  		= "1.0.0"

#-----------------------------------------------------------------
# Game Binary Target Name
#
# Value used for build output filenames, including 
# R36S | Linux | Windows | Web builds
#
# Autostarting on R36S requires a symbolic link named "GameBinaryName" (case-sensitive).
# This value MUST match the symbolic link name on R36S for autostarting to work.
#
# It MUST NOT contain spaces or special characters.
#
# Valid examples:
#   GAME_TARGET = MyGame
#   GAME_TARGET = MyGame_v1
#
# Invalid examples (will break build):
#   GAME_TARGET = "My Game"
#   GAME_TARGET = My\ Game
#-----------------------------------------------------------------
GAME_TARGET			= GameBinaryName

#=================================================================
# SSH Deployment Configuration for R36S
#=================================================================
#-----------------------------------------------------------------
# Modify IP address to match that of 
# R36S ip address
#------------------------------------------------------------------
# R36S username
R36S_USER 			?= ark

# R36S IP address
R36S_HOST 			?= 192.168.68.138

# R36S Game directory path on R36S SD card 
# Device path, verify `lsblk` or `df -h` on R36S
R36S_PATH 			?= /home/ark/autostart

# GDB Server Port
R36S_GDB_PORT  		?= 2345

# Ping timeout in seconds
PING_TIMEOUT 		?= 2												

#=================================================================
# SSH
# Used to Signal SSH
#=================================================================
R36S_CONTROL_SOCKET	:= /tmp/ssh-r36s-control

#-----------------------------------------------------------------
# SSH Explicit Key
# See Dockerfile and scripts/setup_ssh_r36s.sh for SSH key setup
# See ./docs/advanced.md#ssh-setup for more information
#-----------------------------------------------------------------
# Change path to private SSH key for R36S
R36S_SSH_KEY_PRIV	?= /root/.ssh/id_r36s
# Change path to public SSH key for R36S
R36S_SSH_KEY_PUB	?= /root/.ssh/id_r36s.pub

# Copy options
# Files to copy to R36S
CP_SRC_FILES		?= $(PWD)/build/r36s_release/.

# Destination path on R36S SD card
CP_DESTINATION		?= /media/$(shell whoami)/root/home/ark/autostart
