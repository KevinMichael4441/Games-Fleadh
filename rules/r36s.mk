# ----------------------------------------
# R36S specific targets
# ----------------------------------------
.PHONY: __debug_on_r36s __deploy_debug_on_r36s __release_on_r36s __deploy_release_on_r36s

# ----------------------------------------
# Debug R36S build
# ----------------------------------------
__debug_r36s: 	__qemu_test \
				__validate_r36s_platform \
				__validate_debug_config \
				$(TARGET)

	$(call SUCCESS_MSG,Debug target for R36S: $(TARGET))

# ----------------------------------------
# Release R36S build
# ----------------------------------------
__release_r36s: __qemu_test \
				__validate_r36s_platform \
				__validate_release_config \
				$(TARGET)

	$(call SUCCESS_MSG,Release target for R36S: $(TARGET))

# ----------------------------------------
# Start GDB multiarch session on host and 
# Also tasks in VSCode to attach to debug
# session (not implemented yet)
# ----------------------------------------
__debug_on_r36s: __deploy_debug_on_r36s
	$(call INFO_MSG,Starting GDB remote debugging for $(TARGET))
	
	gdb-multiarch ./$(TARGET) \
			-ex "set pagination off" \
			-ex "target remote $(R36S_HOST):$(GDB_PORT)" \
			-ex "set architecture aarch64" \
			-ex "set sysroot /" \
			-ex "set solib-absolute-prefix /usr/aarch64-linux-gnu" \
			-ex "set disable-randomization off" \
			-ex "break main" \
			-ex "continue"
	
	$(call INFO_MSG,$(MSG_DEPLOY_START_ES))
	@ssh -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(R36S_USER)@$(R36S_HOST) "sudo systemctl start emulationstation || true"

# ----------------------------------------
# Deploy and restart gdbserver on R36S
# ----------------------------------------
__deploy_debug_on_r36s: __qemu_test \
						__validate_r36s_platform \
						__validate_debug_config \
						__check_r36s_reachable \
						__check_r36s_ssh \
						__check_ssh_key \
						$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_GDB))
	@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(R36S_USER)@$(R36S_HOST) "\
		sudo pkill -9 gdbserver || true"

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(R36S_USER)@$(R36S_HOST) "\
		sudo systemctl stop emulationstation || true"

	$(call INFO_MSG,$(MSG_DEPLOY_START))
	$(call INFO_MSG,Copying $(TARGET) to R36S at $(R36S_HOST))
	@scp -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(TARGET) \
		$(R36S_USER)@$(R36S_HOST):$(R36S_PATH)
	$(call INFO_MSG,Deployed $(TARGET) to R36S at $(R36S_HOST))
	$(call SUCCESS_MSG,$(MSG_DEPLOY_END))

	$(call INFO_MSG,$(MSG_DEPLOY_START_GDB))
	$(call INFO_MSG,Starting gdbserver on $(R36S_HOST):$(GDB_PORT))
		@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no \
			$(R36S_USER)@$(R36S_HOST) "\
				sudo pkill -9 gdbserver || true; \
				cd $(R36S_PATH) && \
				chmod +x ./$(notdir $(TARGET)) && \
				echo '[INFO] Launching gdbserver on port $(GDB_PORT)...'; \
				nohup sudo -E /usr/bin/gdbserver :$(GDB_PORT) ./$(notdir $(TARGET)) > /dev/null 2>&1 & \
				sleep 2; \
				if sudo netstat -tlnp 2>/dev/null | grep -q $(GDB_PORT); then \
					echo '[OK] gdbserver is running.'; \
				else \
					echo '[WARN] gdbserver not detected!'; \
				fi; \
				exit 0" >/dev/null
	$(call SUCCESS_MSG,gdbserver started on $(R36S_HOST):$(GDB_PORT))

# ----------------------------------------
# Release on R36S
# ----------------------------------------
__release_on_r36s: __deploy_release_on_r36s
	$(call INFO_MSG,$(MSG_DEPLOY_START_ES))
	$(call INFO_MSG,Starting $(TARGET) on R36S at $(R36S_HOST))
	@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(R36S_USER)@$(R36S_HOST) "\
		chmod +x $(R36S_PATH)/$(notdir $(TARGET)) && \
		cd $(R36S_PATH) && \
		echo '[INFO] Running $(notdir $(TARGET))...' && \
		./$(notdir $(TARGET)); \
		echo '[INFO] Restarting frontend...'; \
		sudo systemctl start emulationstation || true"

# ----------------------------------------
# Deploy Release on R36S
# ----------------------------------------
__deploy_release_on_r36s: 	__qemu_test \
							__validate_r36s_platform \
							__validate_release_config \
							__check_r36s_reachable \
							__check_r36s_ssh \
							__check_ssh_key \
							$(TARGET)

	$(call INFO_MSG,Deploying $(TARGET) to R36S at $(R36S_HOST))

	$(call INFO_MSG,Stop any running gdbserver)
	@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=accept-new $(R36S_USER)@$(R36S_HOST) "sudo pkill -9 gdbserver || true"

	$(call INFO_MSG,$(MSG_DEPLOY_STOP_ES))
	@ssh -T -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=accept-new $(R36S_USER)@$(R36S_HOST) "sudo systemctl stop emulationstation || true"

	$(call INFO_MSG,$(MSG_DEPLOY_COPY))
	$(call INFO_MSG,Copy $(TARGET) to R36S at $(R36S_HOST))
	@scp -i $(R36S_SSH_KEY) -o StrictHostKeyChecking=no $(TARGET) $(R36S_USER)@$(R36S_HOST):$(R36S_PATH)
	$(call INFO_MSG,Deployed $(TARGET) to R36S at $(R36S_HOST))
	$(call SUCCESS_MSG,$(MSG_DEPLOY_END))

# ----------------------------------------
# Check Connection and SSH key access to 
# R36S
# ----------------------------------------
.PHONY: __check_r36s_reachable __check_r36s_ssh __check_ssh_key

__check_r36s_reachable:
	$(call INFO_LINE,Checking if R36S HOST is reachable: $(R36S_HOST))
	@ping -c 1 -W $(PING_TIMEOUT) $(R36S_HOST) >/dev/null 2>&1 && { \
		$(call SUCCESS_LINE,R36S HOST responds to ping: $(R36S_HOST)); \
		exit 0; \
	} || true
	$(call WARN_LINE,Ping failed. Trying SSH reachability instead...)
	@ssh -i $(R36S_SSH_KEY) \
		-o BatchMode=yes \
		-o StrictHostKeyChecking=accept-new \
		-o ConnectTimeout=$(CONNECT_TIMEOUT) \
		$(R36S_USER)@$(R36S_HOST) "echo ok" >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_r36s_reachable_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_LINE,R36S HOST reachable via SSH: $(R36S_HOST))

__check_r36s_reachable_failed:
	$(call ERROR_MSG,$(MSG_R36S_CONNECT_ERROR)\n Current host set to: $(R36S_HOST))
	@false

__check_r36s_ssh:
	$(call INFO_MSG,Checking SSH on R36S HOST is reachable: $(R36S_USER)@$(R36S_HOST))
	@ssh -i $(R36S_SSH_KEY) \
		-o BatchMode=yes \
		-o StrictHostKeyChecking=no \
		-o ConnectTimeout=$(CONNECT_TIMEOUT) \
		$(R36S_USER)@$(R36S_HOST) "echo ok" >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_r36s_ssh_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_MSG,SSH on R36S HOST is reachable: $(R36S_USER)@$(R36S_HOST))

__check_r36s_ssh_failed:
	$(call ERROR_MSG,$(MSG_R36S_CONNECT_ERROR))
	@false

__check_ssh_key:
	$(call INFO_MSG,$(R36S_HOST) $(MSG_SSH_CHECK))
	@ssh -i $(R36S_SSH_KEY) -o BatchMode=yes $(R36S_USER)@$(R36S_HOST) "echo 'SSH key works.'" >/dev/null 2>&1 || { \
		$(MAKE) -f $(firstword $(MAKEFILE_LIST)) --no-print-directory __check_ssh_key_failed 2>/dev/null; \
		exit 1; \
	}
	$(call SUCCESS_MSG,$(MSG_SSH_KEY_SUCCESS))

__check_ssh_key_failed:
	$(call ERROR_MSG,$(MSG_SSH_KEY_ERROR))
	@false