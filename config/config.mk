#=================================================================
# Game Configuration
# Carefully modify values below to match your game configuration
# For game name, description, version, path, and module name
# use escape characters for spaces and special characters, 
# e.g. "My\ Game\ Name" instead of "My Game Name"
# e.g. "A\ brief\ description\ of\ the\ game." 
# instead of "A brief description of the game."
# Special characters like '' and '"' should be escaped
# e.g. The game\'s is a backslash \\ away from a \; quote \" character.
#=================================================================
GAME_NAME     		= Game\ Name
GAME_DESCRIPTION	= A\ brief\ description\ of\ the\ game.
GAME_VERSION  		= 1.0.0
# Game target no spaces or special characters, used for build output naming
# For example, if GAME_TARGET is "SomeGame"
# The binary will be named "SomeGame_platform_config"
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
R36S_HOST 			?= 192.168.0.0

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
