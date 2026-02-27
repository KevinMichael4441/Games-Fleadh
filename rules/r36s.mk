#=================================================================
# R36S specific targets
#=================================================================
.PHONY: __debug_r36s \
		__release_r36s \
		__debug_on_r36s_with_ssh_default \
		__debug_on_r36s_with_ssh_explicit_key \
		__deploy_debug_on_r36s_with_ssh_default \
		__deploy_debug_on_r36s_with_ssh_explicit_key \
		__release_on_r36s_with_ssh_default \
		__release_on_r36s_with_ssh_explicit_key \
		__deploy_release_on_r36s_with_ssh_default \
		__deploy_release_on_r36s_with_ssh_explicit_key \
		__r36s_gdbserver_ensure_default \
        __r36s_gdbserver_ensure_explicit_key \
		__r36s_ssh_ensure \
		__r36s_user_validate \
		__stop_r36s_target

#=================================================================
# Stable alias for autostart/debug/deploy on device
# NOTE:
# - Keep this constant (do NOT tie to TARGET_NAME) so systemd/autostart
#   never changes even if the binary name changes.
#=================================================================
R36S_GAME_ALIAS ?= game_binary

#=================================================================
# SSH options (Control Socket / ControlMaster / Explicit Key)
#=================================================================

# NOTES: Default SSH
# - ssh uses:   -S <control_socket>
SSH_DEFAULT_OPTS = -T \
	-o ControlMaster=auto \
	-o ControlPath="$(R36S_CONTROL_SOCKET)" \
	-o ControlPersist=10m \
	-o StrictHostKeyChecking=no \
	-o UserKnownHostsFile=/dev/null

#-----------------------------------------------------------------
# NOTES: Default SCP
# - scp uses:   -o ControlPath=<control_socket>
# - scp does NOT support -T
# - -o ControlPath="$(R36S_CONTROL_SOCKET)" file must exist
# - ControlPersist=10m extends socket lifetime
#-----------------------------------------------------------------
SCP_DEFAULT_OPTS = \
	-o ControlMaster=auto \
	-o ControlPath="$(R36S_CONTROL_SOCKET)" \
	-o ControlPersist=10m \
	-o StrictHostKeyChecking=no \
	-o UserKnownHostsFile=/dev/null

#-----------------------------------------------------------------
# NOTES: Default RSYNC
# - $(TARGET) is the built binary (file)
# - rsync source is the directory containing $(TARGET)
# - specify excludes (obj + common build artifacts)
# - --delete keeps destination in sync (remove old files)
#-----------------------------------------------------------------
RSYNC_DEFAULT_OPTS = -av --delete \
	--exclude 'obj/' \
	--exclude '*.o' \
	--exclude '*.a'

# rsync uses ssh via -e "ssh ..."
RSYNC_SSH_DEFAULT = ssh \
	-o ControlMaster=auto \
	-o ControlPath="$(R36S_CONTROL_SOCKET)" \
	-o ControlPersist=10m \
	-o StrictHostKeyChecking=no \
	-o UserKnownHostsFile=/dev/null

#-----------------------------------------------------------------
# NOTES: Explicit Key SSH
# - ssh uses:   -o ControlMaster=no
# - -i $(R36S_SSH_KEY_PRIV) public key must exist on R36S
# - ControlMaster=no avoids multiplex socket conflicts
# - ConnectTimeout stops hanging (R36S asleep/offline)
# - BatchMode=yes fails fast instead of prompting
#-----------------------------------------------------------------
SSH_EXPLICIT_KEY_OPTS = -T \
	-i "$(R36S_SSH_KEY_PRIV)" \
	-o ControlMaster=no \
	-o StrictHostKeyChecking=accept-new \
	-o UserKnownHostsFile=/dev/null \
	-o ConnectTimeout="$(CONNECT_TIMEOUT)" \
	-o BatchMode=yes

#-----------------------------------------------------------------
# NOTES: Explicit Key SCP
# - scp uses:   -o ControlMaster=no
# - scp does NOT support -T
# - -i $(R36S_SSH_KEY_PRIV) public key must exist on R36S
# - ControlMaster=no avoids multiplex socket conflicts
# - ConnectTimeout stops hanging (R36S asleep/offline)
# - BatchMode=yes fails fast instead of prompting
#-----------------------------------------------------------------
SCP_EXPLICIT_KEY_OPTS = \
	-i "$(R36S_SSH_KEY_PRIV)" \
	-o ControlMaster=no \
	-o StrictHostKeyChecking=accept-new \
	-o UserKnownHostsFile=/dev/null \
	-o ConnectTimeout="$(CONNECT_TIMEOUT)" \
	-o BatchMode=yes

#-----------------------------------------------------------------
# NOTES: Explicit Key RSYNC
# - specify excludes (obj + common build artifacts)
# - --delete keeps destination in sync (remove old files)
#-----------------------------------------------------------------
RSYNC_EXPLICIT_KEY_OPTS = -av --delete \
	--exclude 'obj/' \
	--exclude '*.o' \
	--exclude '*.a'

RSYNC_SSH_EXPLICIT_KEY = ssh $(SSH_EXPLICIT_KEY_OPTS)

#-----------------------------------------------------------------
# DEPLOY SOURCE DIRECTORY
# - If $(TARGET) is a file (recommended), this resolves to its folder
# - If $(TARGET) is already a folder, $(dir ...) still works (returns itself)
# - Trailing slash is added at rsync call-site to copy *contents*
#-----------------------------------------------------------------
TARGET_DIR := $(dir $(TARGET))

#=================================================================
# Debug R36S build
#=================================================================
__debug_r36s: 	__qemu_test \
				__validate_r36s_platform \
				__validate_debug_config \
				$(TARGET)

	$(call SUCCESS_MSG,Debug target for R36S: $(TARGET))

#=================================================================
# Release R36S build
#=================================================================
__release_r36s: __qemu_test \
				__validate_r36s_platform \
				__validate_release_config \
				$(TARGET)

	$(call SUCCESS_MSG,Release target for R36S: $(TARGET))

#=================================================================
# R36S User must be set
#=================================================================
__r36s_user_validate:
	@if [ -z "$(R36S_USER)" ]; then \
		$(call ERROR_LINE,R36S_USER is not set. Check ./config/config.mk); \
		exit 1; \
	fi

#=================================================================
# SSH to R36S via Control Socket
#=================================================================
# IMPORTANT:
# - Socket file existing is not enough; it can be stale.
# - "ssh -O check" verifies the control master is actually alive.
# NOTE:
# This target is ONLY valid for password-based SSH
# Explicit-key targets must NOT depend on this
#=================================================================
__r36s_ssh_ensure:
	@ssh $(SSH_DEFAULT_OPTS) -O check $(R36S_USER)@$(R36S_HOST) >/dev/null 2>&1 || { \
		$(call INFO_MSG,SSH control socket stale or missing. Reconnecting...); \
		rm -f "$(R36S_CONTROL_SOCKET)" >/dev/null 2>&1 || true; \
		R36S_CONTROL_SOCKET="$(R36S_CONTROL_SOCKET)" R36S_USER="$(R36S_USER)" R36S_HOST="$(R36S_HOST)" bash scripts/r36s_ssh_connection_up.sh; \
	}

#=================================================================
# Stop any running instance of deployed target
# Prevents: rsync / chmod overwriting a running binary
#=================================================================
__stop_r36s_target: __r36s_ssh_ensure
	$(call INFO_MSG,Stopping running $(R36S_GAME_ALIAS) if present...)
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		if pgrep -f '(^|/)\./$(R36S_GAME_ALIAS)( |$$)' >/dev/null 2>&1 || pgrep -x '$(R36S_GAME_ALIAS)' >/dev/null 2>&1; then \
			echo 'Stopping $(R36S_GAME_ALIAS)...'; \
			sudo pkill -TERM -f '(^|/)\./$(R36S_GAME_ALIAS)( |$$)' || true; \
			sudo pkill -TERM -x '$(R36S_GAME_ALIAS)' || true; \
			sleep 0.5; \
			sudo pkill -KILL -f '(^|/)\./$(R36S_GAME_ALIAS)( |$$)' || true; \
			sudo pkill -KILL -x '$(R36S_GAME_ALIAS)' || true; \
		else \
			echo 'No $(R36S_GAME_ALIAS) process running'; \
		fi"

#=================================================================
# Ensure gdbserver is installed (SSH Default)
#=================================================================
__r36s_gdbserver_ensure_default: __r36s_user_validate __r36s_ssh_ensure
	$(call INFO_MSG,Ensuring gdbserver is installed on R36S (SSH default))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		if command -v gdbserver >/dev/null 2>&1; then \
			echo '[OK] gdbserver already installed.'; \
		else \
			echo '[INFO] gdbserver not found. Installing...'; \
			sudo apt-get update -y && sudo apt-get install -y gdbserver; \
		fi"
	$(call SUCCESS_MSG,gdbserver is installed on R36S)

#=================================================================
# Ensure gdbserver is installed (SSH explicit key)
#=================================================================
__r36s_gdbserver_ensure_explicit_key:
	$(call INFO_MSG,Ensuring gdbserver is installed on R36S (SSH explicit key))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		if command -v gdbserver >/dev/null 2>&1; then \
			echo '[OK] gdbserver already installed.'; \
		else \
			echo '[INFO] gdbserver not found. Installing...'; \
			sudo apt-get update -y && sudo apt-get install -y gdbserver; \
		fi"
	$(call SUCCESS_MSG,gdbserver is installed on R36S)

#=================================================================
# Start GDB multiarch session on host and 
# Also tasks in VSCode to attach to debug
# session (not implemented yet) with
# SSH (password only, no key)
#
#-----------------------------------------------------------------
# WARNING:
#-----------------------------------------------------------------
# This means R36S is exposed on
# your network with default username and 
# password. At a minimum it is recommended
# to change default 'ark' password
# Changing password and implementing PPK is
# highly is recommended 
#=================================================================
__debug_on_r36s_with_ssh_default: __deploy_debug_on_r36s_with_ssh_default
	$(call INFO_MSG,Starting GDB remote debugging for $(TARGET))

	gdb-multiarch ./$(TARGET) \
			-ex "set pagination off" \
			-ex "target remote $(R36S_HOST):$(R36S_GDB_PORT)" \
			-ex "set architecture aarch64" \
			-ex "set sysroot /" \
			-ex "set solib-absolute-prefix /usr/aarch64-linux-gnu" \
			-ex "set disable-randomization off" \
			-ex "break main" \
			-ex "continue"

	$(call INFO_MSG,$(MSG_DEPLOY_START_ES))

#=================================================================
# Start GDB multiarch session on host and 
# Also tasks in VSCode to attach to debug
# session (not implemented yet) with
# Public and Private Key (PPK)
# See ./docs/advanced.md
#=================================================================
__debug_on_r36s_with_ssh_explicit_key: __deploy_debug_on_r36s_with_ssh_explicit_key
	$(call INFO_MSG,Starting GDB remote debugging for $(TARGET))

	gdb-multiarch ./$(TARGET) \
			-ex "set pagination off" \
			-ex "target remote $(R36S_HOST):$(R36S_GDB_PORT)" \
			-ex "set architecture aarch64" \
			-ex "set sysroot /" \
			-ex "set solib-absolute-prefix /usr/aarch64-linux-gnu" \
			-ex "set disable-randomization off" \
			-ex "break main" \
			-ex "continue"

	$(call INFO_MSG,$(MSG_DEPLOY_START_ES))

#=================================================================
# Deploy and restart gdbserver on R36S with
# SSH (password only, no key)
#
#-----------------------------------------------------------------
# WARNING:
#-----------------------------------------------------------------
# This means R36S is exposed on
# your network with default username and 
# password. At a minimum it is recommended
# to change default 'ark' password
# Changing password and implementing PPK is
# highly is recommended 
#=================================================================
__deploy_debug_on_r36s_with_ssh_default: __qemu_test \
						__validate_r36s_platform \
						__validate_debug_config \
						__check_r36s_reachable \
						__r36s_user_validate \
						__r36s_gdbserver_ensure_default \
						$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	# Ensure destination exists
	$(call INFO_MSG,Ensuring R36S_PATH exists: $(R36S_PATH))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"mkdir -p '$(R36S_PATH)'"

	# Stop running binary (prevents overwriting a running executable)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __stop_r36s_target

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_GDB))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo pkill -9 gdbserver || true"

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo systemctl stop emulationstation || true"

	#-----------------------------------------------------------------
	# RSYNC DEPLOY (directory sync)
	# - copies binary + assets folder(s)
	# - excludes obj/ etc
	# - removes stale files on destination (--delete)
	#-----------------------------------------------------------------
	$(call INFO_MSG,$(MSG_DEPLOY_START))
	$(call INFO_MSG,Rsync $(TARGET_DIR) -> $(R36S_USER)@$(R36S_HOST):$(R36S_PATH))
	@rsync $(RSYNC_DEFAULT_OPTS) \
		-e "$(RSYNC_SSH_DEFAULT)" \
		"$(TARGET_DIR)/" \
		"$(R36S_USER)@$(R36S_HOST):$(R36S_PATH)/"

	# Ensure binary is executable
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"chmod +x '$(R36S_PATH)/$(notdir $(TARGET))' || true"

	# Create/refresh stable symlink on device (game_binary -> deployed binary)
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		cd '$(R36S_PATH)' && \
		ln -sfn '$(notdir $(TARGET))' '$(R36S_GAME_ALIAS)' && \
		chmod +x '$(R36S_GAME_ALIAS)' || true && \
		ls -l '$(R36S_GAME_ALIAS)' || true"

	$(call INFO_MSG,$(MSG_DEPLOY_START_GDB))
	$(call INFO_MSG,Starting gdbserver on $(R36S_HOST):$(R36S_GDB_PORT))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		sudo pkill -9 gdbserver || true; \
		cd '$(R36S_PATH)' && \
		echo '[INFO] Launching gdbserver on port $(R36S_GDB_PORT)...'; \
		nohup sudo -E /usr/bin/gdbserver :$(R36S_GDB_PORT) ./$(R36S_GAME_ALIAS) > /dev/null 2>&1 & \
		sleep 2; \
		exit 0" >/dev/null

	$(call SUCCESS_MSG,gdbserver started on $(R36S_HOST):$(R36S_GDB_PORT))

#=================================================================
# Deploy and restart gdbserver on R36S with
# Public and Private Key (PPK)
# See ./docs/advanced.md
#=================================================================
__deploy_debug_on_r36s_with_ssh_explicit_key: __qemu_test \
						__validate_r36s_platform \
						__validate_debug_config \
						__check_r36s_reachable \
						__check_r36s_ssh_service_explicit_key \
						__check_r36s_ssh_explicit_key \
						__r36s_gdbserver_ensure_explicit_key \
						$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	# Ensure destination exists
	$(call INFO_MSG,Ensuring R36S_PATH exists: $(R36S_PATH))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"mkdir -p '$(R36S_PATH)'"

	# Stop running binary (prevents overwriting a running executable)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __stop_r36s_target

	# Stop GDB Server
	$(call INFO_MSG,$(MSG_DEPLOY_STOP_GDB))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo pkill -9 gdbserver || true"

	# Stop EmulationStation
	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo systemctl stop emulationstation || true"

	#-----------------------------------------------------------------
	# RSYNC DEPLOY (directory sync)
	# - copies binary + assets folder(s)
	# - excludes obj/ etc
	# - removes stale files on destination (--delete)
	#-----------------------------------------------------------------
	$(call INFO_MSG,$(MSG_DEPLOY_START))
	$(call INFO_MSG,Rsync $(TARGET_DIR) -> $(R36S_USER)@$(R36S_HOST):$(R36S_PATH))
	@rsync $(RSYNC_EXPLICIT_KEY_OPTS) \
		-e "$(RSYNC_SSH_EXPLICIT_KEY)" \
		"$(TARGET_DIR)/" \
		"$(R36S_USER)@$(R36S_HOST):$(R36S_PATH)/"

	# Ensure binary is executable
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"chmod +x '$(R36S_PATH)/$(notdir $(TARGET))' || true"

	# Create/refresh stable symlink on device (game_binary -> deployed binary)
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		cd '$(R36S_PATH)' && \
		ln -sfn '$(notdir $(TARGET))' '$(R36S_GAME_ALIAS)' && \
		chmod +x '$(R36S_GAME_ALIAS)' || true && \
		ls -l '$(R36S_GAME_ALIAS)' || true"

	$(call INFO_MSG,Deployed $(TARGET) to R36S at $(R36S_HOST))
	$(call SUCCESS_MSG,$(MSG_DEPLOY_END))

	# Start GDB
	$(call INFO_MSG,$(MSG_DEPLOY_START_GDB))
	$(call INFO_MSG,Starting gdbserver on $(R36S_HOST):$(R36S_GDB_PORT))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		sudo pkill -9 gdbserver || true; \
		cd '$(R36S_PATH)' && \
		echo '[INFO] Launching gdbserver on port $(R36S_GDB_PORT)...'; \
		nohup sudo -E /usr/bin/gdbserver :$(R36S_GDB_PORT) ./$(R36S_GAME_ALIAS) > /dev/null 2>&1 & \
		sleep 2; \
		if sudo netstat -tlnp 2>/dev/null | grep -q $(R36S_GDB_PORT); then \
			echo '[OK] gdbserver is running.'; \
		else \
			echo '[WARN] gdbserver not detected!'; \
		fi; \
		exit 0" >/dev/null

	$(call SUCCESS_MSG,gdbserver started on $(R36S_HOST):$(R36S_GDB_PORT))

#=================================================================
# Release on R36S with
# SSH (password only, no key)
#
#-----------------------------------------------------------------
# WARNING:
#-----------------------------------------------------------------
# This means R36S is exposed on
# your network with default username and 
# password. At a minimum it is recommended
# to change default 'ark' password
# Changing password and implementing PPK is
# highly is recommended 
#=================================================================
__release_on_r36s_with_ssh_default: __deploy_release_on_r36s_with_ssh_default

	$(call INFO_MSG,Starting $(TARGET) on R36S at $(R36S_HOST))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		chmod +x '$(R36S_PATH)/$(R36S_GAME_ALIAS)' && \
		cd '$(R36S_PATH)' && \
		echo '[INFO] Running $(R36S_GAME_ALIAS)...' && \
		./$(R36S_GAME_ALIAS)"

#=================================================================
# Release on R36S with
# Public and Private Key (PPK)
# See ./docs/advanced.md
#=================================================================
__release_on_r36s_with_ssh_explicit_key: __deploy_release_on_r36s_with_ssh_explicit_key

	$(call INFO_MSG,Starting $(TARGET) on R36S at $(R36S_HOST))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		chmod +x '$(R36S_PATH)/$(R36S_GAME_ALIAS)' && \
		cd '$(R36S_PATH)' && \
		echo '[INFO] Running $(R36S_GAME_ALIAS)...' && \
		./$(R36S_GAME_ALIAS)"

#=================================================================
# Deploy Release on R36S with 
# SSH (password only, no key)
#
#-----------------------------------------------------------------
# WARNING:
#-----------------------------------------------------------------
# This means R36S is exposed on
# your network with default username and 
# password. At a minimum it is recommended
# to change default 'ark' password
# Changing password and implementing PPK is
# highly is recommended 
#=================================================================
__deploy_release_on_r36s_with_ssh_default: __qemu_test \
											__validate_r36s_platform \
											__validate_release_config \
											__check_r36s_reachable \
											__r36s_user_validate \
											__r36s_ssh_ensure \
											$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	# Ensure destination exists
	$(call INFO_MSG,Ensuring R36S_PATH exists: $(R36S_PATH))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"mkdir -p '$(R36S_PATH)'"

	# Stop running binary (prevents overwriting a running executable)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __stop_r36s_target

	# Stop any running gdbserver
	$(call INFO_MSG,Stop any running gdbserver)
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo pkill -9 gdbserver || true"

	# Stop EmulationStation (optional but recommended for framebuffer focus)
	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo systemctl stop emulationstation || true"

	#-----------------------------------------------------------------
	# RSYNC DEPLOY (directory sync)
	# - copies binary + assets folder(s)
	# - excludes obj/ etc
	# - removes stale files on destination (--delete)
	#-----------------------------------------------------------------
	$(call INFO_MSG,$(MSG_DEPLOY_COPY))
	$(call INFO_MSG,Rsync $(TARGET_DIR) -> $(R36S_USER)@$(R36S_HOST):$(R36S_PATH))
	@rsync $(RSYNC_DEFAULT_OPTS) \
		-e "$(RSYNC_SSH_DEFAULT)" \
		"$(TARGET_DIR)/" \
		"$(R36S_USER)@$(R36S_HOST):$(R36S_PATH)/"

	# Ensure binary is executable
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"chmod +x '$(R36S_PATH)/$(notdir $(TARGET))' || true"

	# Create/refresh stable symlink on device (game_binary -> deployed binary)
	@ssh $(SSH_DEFAULT_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		cd '$(R36S_PATH)' && \
		ln -sfn '$(notdir $(TARGET))' '$(R36S_GAME_ALIAS)' && \
		chmod +x '$(R36S_GAME_ALIAS)' || true && \
		ls -l '$(R36S_GAME_ALIAS)' || true"

	$(call INFO_MSG,Deployed $(TARGET) to R36S at $(R36S_HOST))
	$(call SUCCESS_MSG,$(MSG_DEPLOY_END))

#=================================================================
# Deploy Release on R36S with 
# Public and Private Key (PPK)
# See ./docs/advanced.md
#=================================================================
__deploy_release_on_r36s_with_ssh_explicit_key: __qemu_test \
							__validate_r36s_platform \
							__validate_release_config \
							__check_r36s_reachable \
							__check_r36s_ssh_service_explicit_key \
							__check_r36s_ssh_explicit_key \
							$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	# Ensure destination exists
	$(call INFO_MSG,Ensuring R36S_PATH exists: $(R36S_PATH))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"mkdir -p '$(R36S_PATH)'"

	# Stop running binary (prevents overwriting a running executable)
	@$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __stop_r36s_target

	$(call INFO_MSG,Stop any running gdbserver)
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo pkill -9 gdbserver || true"

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"sudo systemctl stop emulationstation || true"

	#-----------------------------------------------------------------
	# RSYNC DEPLOY (directory sync)
	# - copies binary + assets folder(s)
	# - excludes obj/ etc
	# - removes stale files on destination (--delete)
	#-----------------------------------------------------------------
	$(call INFO_MSG,$(MSG_DEPLOY_COPY))
	$(call INFO_MSG,Rsync $(TARGET_DIR) -> $(R36S_USER)@$(R36S_HOST):$(R36S_PATH))
	@rsync $(RSYNC_EXPLICIT_KEY_OPTS) \
		-e "$(RSYNC_SSH_EXPLICIT_KEY)" \
		"$(TARGET_DIR)/" \
		"$(R36S_USER)@$(R36S_HOST):$(R36S_PATH)/"

	# Ensure binary is executable
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"chmod +x '$(R36S_PATH)/$(notdir $(TARGET))' || true"

	# Create/refresh stable symlink on device (game_binary -> deployed binary)
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) "\
		cd '$(R36S_PATH)' && \
		ln -sfn '$(notdir $(TARGET))' '$(R36S_GAME_ALIAS)' && \
		chmod +x '$(R36S_GAME_ALIAS)' || true && \
		ls -l '$(R36S_GAME_ALIAS)' || true"

	$(call INFO_MSG,Deployed $(TARGET) to R36S at $(R36S_HOST))
	$(call SUCCESS_MSG,$(MSG_DEPLOY_END))

#=================================================================
# Check Connection and SSH key access for 
# R36S connected to same network as Host
#=================================================================
.PHONY: __check_r36s_reachable \
		__check_r36s_ssh_service_explicit_key \
		__check_r36s_ssh_explicit_key \
		__check_r36s_ssh_explicit_key_failed

#=================================================================
# Verify R36S is reachable using SSH
#=================================================================
__check_r36s_reachable:

	$(call INFO_LINE,Checking if R36S HOST is reachable (ping): $(R36S_HOST))
	@ping -4 -c 1 -W $(PING_TIMEOUT) $(R36S_HOST) >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_r36s_reachable_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_LINE,R36S HOST responds to ping: $(R36S_HOST))

__check_r36s_reachable_failed:

	$(call ERROR_MSG,$(MSG_R36S_CONNECT_ERROR)\n Ping failed.\n Current host set to: $(R36S_HOST))
	@false

#=================================================================
# Verify SSH service available using key
#=================================================================
__check_r36s_ssh_service_explicit_key:

	$(call INFO_MSG,Checking SSH on R36S HOST is reachable: $(R36S_USER)@$(R36S_HOST))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"echo ok" >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_r36s_ssh_service_explicit_key_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_MSG,SSH on R36S HOST is reachable: $(R36S_USER)@$(R36S_HOST))

__check_r36s_ssh_service_explicit_key_failed:

	$(call ERROR_MSG,$(MSG_R36S_SSH_EK_ERROR))
	@false

#=================================================================
# Verify key works
#=================================================================
__check_r36s_ssh_explicit_key:

	$(call INFO_MSG,$(R36S_HOST) $(MSG_SSH_CHECK))
	@ssh $(SSH_EXPLICIT_KEY_OPTS) $(R36S_USER)@$(R36S_HOST) \
		"echo 'SSH key works.'" >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_r36s_ssh_explicit_key_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_MSG,$(MSG_SSH_KEY_SUCCESS))

__check_r36s_ssh_explicit_key_failed:

	$(call ERROR_MSG,$(MSG_SSH_KEY_ERROR))
	@false