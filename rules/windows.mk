#=================================================================
# TODO Windows makefile rules
# Windows specific makefile rules for MinGW
#=================================================================
.PHONY: __debug_windows __release_windows

#=================================================================
# Environment fallback variable forcing rendering with Wine
#=================================================================
WINE ?= LIBGL_ALWAYS_SOFTWARE=1 wine

__debug_windows: __validate_windows_platform __validate_debug_config $(TARGET)
	$(call INFO_MSG,Starting Debug $(TARGET) on Windows)
	$(call INFO_MSG,Run on Windows or with: wine ./$(TARGET)$(TARGET_EXT))

__release_windows: __validate_windows_platform __validate_release_config $(TARGET)
	$(call INFO_MSG,Starting Release $(TARGET) on Windows)

ifeq ($(HOST_IS_WINDOWS),TRUE)
	$(call INFO_MSG,Run on Windows ./$(TARGET)$(TARGET_EXT))
	@true
else
	$(call INFO_MSG,Run on Windows or with: wine ./$(TARGET)$(TARGET_EXT))
	$(WINE) ./$(TARGET)$(TARGET_EXT)
endif