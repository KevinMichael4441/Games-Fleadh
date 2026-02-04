# SSH Deployment Configuration for R36S
R36S_USER 			?= ark 												# Change to R36S username
R36S_HOST 			?= 192.168.0.61 									# Change to R36S IP address
R36S_PATH 			?= /home/ark/autostart 								# Change to path on R36S
GDB_PORT  			?= 2345												# GDB Server Port
PING_TIMEOUT 		?= 2												# Ping timeout in seconds

# SSH Key
# See Dockerfile and scripts/setup_ssh_r36s.sh for SSH key setup
# See ./docs/advanced.md#ssh-setup for more information
R36S_SSH_KEY		?= /root/.ssh/id_r36s

# Copy options
CP_SRC_FILES		?= $(PWD)/build/r36s_release/.						# Files to copy to R36S
CP_DESTINATION		?= /media/$(shell whoami)/root/home/ark/autostart	# Destination path on R36S SD card
