#=================================================================
# Validation Targets
#=================================================================
.PHONY: __validate_config_value __validate_platform_value __validate_debug_config \
        __validate_release_config __validate_r36s_platform __validate_linux_platform \
        __validate_web_platform __validate_windows_platform

#=================================================================
# Validate config is a valid value
#=================================================================
__validate_config_value:
	@$(if $(filter $(CONFIG),$(DEBUG_BUILD) $(RELEASE_BUILD)),,\
		$(call WARNING_MSG,$(MSG_HELP_PLATFORM)); \
		$(call ERROR_MSG,Invalid CONFIG "$(CONFIG)"); \
		exit 1)

#=================================================================
# Validate platform is a valid value
#=================================================================
__validate_platform_value:
	@$(if $(filter $(PLATFORM),$(R36S_TARGET) $(LINUX_TARGET) $(WEB_TARGET) $(WINDOWS_TARGET)),,\
		$(call WARNING_MSG,$(MSG_HELP_PLATFORM)); \
		$(call ERROR_MSG,Invalid PLATFORM "$(PLATFORM)"); \
		exit 1)

#=================================================================
# Validate CONFIG is set to debug
#=================================================================
__validate_debug_config:
	@$(if $(filter $(DEBUG_BUILD),$(CONFIG)),,\
		$(call WARNING_MSG,$(MSG_INVALID_CONFIG_DEBUG)); \
		$(call ERROR_MSG,Current CONFIG is "$(CONFIG)"); \
		exit 1)

#=================================================================
# Validate CONFIG is set to release
#=================================================================
__validate_release_config:
	@$(if $(filter $(RELEASE_BUILD),$(CONFIG)),,\
		$(call WARNING_MSG,$(MSG_INVALID_CONFIG_RELEASE)); \
		$(call ERROR_MSG,Current CONFIG is "$(CONFIG)"); \
		exit 1)

#=================================================================
# Validate PLATFORM is set to r36s
#=================================================================
__validate_r36s_platform:
	@$(if $(filter $(R36S_TARGET),$(PLATFORM)),,\
		$(call WARNING_MSG,$(MSG_USAGE_CONFIG_R36S)); \
		$(call ERROR_MSG,Current PLATFORM is "$(PLATFORM)"); \
		exit 1)

#=================================================================
# Validate PLATFORM is set to linux
#=================================================================
__validate_linux_platform:
	@$(if $(filter $(LINUX_TARGET),$(PLATFORM)),,\
		$(call WARNING_MSG,$(MSG_USAGE_CONFIG_LINUX)); \
		$(call ERROR_MSG,Current PLATFORM is "$(PLATFORM)"); \
		exit 1)

#=================================================================
# Validate PLATFORM is set to web
#=================================================================
__validate_web_platform:
	@$(if $(filter $(WEB_TARGET),$(PLATFORM)),,\
		$(call WARNING_MSG,$(MSG_USAGE_CONFIG_WEB)); \
		$(call ERROR_MSG,Current PLATFORM is "$(PLATFORM)"); \
		exit 1)

#=================================================================
# Validate PLATFORM is set to windows
#=================================================================
__validate_windows_platform:
	@$(if $(filter $(WINDOWS_TARGET),$(PLATFORM)),,\
		$(call WARNING_MSG,$(MSG_USAGE_CONFIG_WINDOWS)); \
		$(call ERROR_MSG,Current PLATFORM is "$(PLATFORM)"); \
		exit 1)