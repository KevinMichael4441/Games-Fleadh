#=================================================================
# Message Color Codes
#=================================================================
BLUE		:= \033[0;34m
GREEN		:= \033[0;32m
YELLOW		:= \033[0;33m
RED			:= \033[0;31m

#=================================================================
# Arrow symbols
#=================================================================
UP_ARROW	:= \xE2\x96\xB2
DOWN_ARROW	:= \xE2\x96\xBC

RESET		:= \033[0m

#=================================================================
# Platform detection
#=================================================================
ifeq ($(OS),Windows_NT)
	IS_WINDOWS := TRUE
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		IS_MACOS := TRUE
	else
		IS_LINUX := TRUE
	endif
endif

#=================================================================
# Color output function for macOS (does not require -e flag)
#=================================================================
define PRINT_MSG_MACOS
	echo "$(1)$(2)$(RESET)"
endef

#=================================================================
# Color output function for Linux (uses /bin/echo -e)
#=================================================================
define PRINT_MSG_LINUX
	/bin/echo -e "$(1)$(2)$(RESET)"
endef

#=================================================================
# Color output function for Windows
#=================================================================
define PRINT_MSG_WIN
	/bin/echo -e "$(1)$(2)$(RESET)"
endef

#=================================================================
# Platform-specific message printing logic
#=================================================================
ifeq ($(IS_WINDOWS),TRUE)
  PRINT_MSG = $(PRINT_MSG_WIN)
else ifeq ($(IS_MACOS),TRUE)
  PRINT_MSG = $(PRINT_MSG_MACOS)
else ifeq ($(IS_LINUX),TRUE)
  PRINT_MSG = $(PRINT_MSG_LINUX)
endif

#=================================================================
# Message macros
#=================================================================
define INFO_MSG
	$(call PRINT_MSG,$(BLUE),INFO: $(1))
endef
define INFO_MSG_HEAD
	$(call PRINT_MSG,$(BLUE),**********************************************************************)
	$(call PRINT_MSG,$(BLUE),INFO: $(1))
	$(call PRINT_MSG,$(BLUE),**********************************************************************)
endef
define INFO_MSG_BOX
	$(call PRINT_MSG,$(BLUE),$(DOWN_ARROW)********************************************************************$(DOWN_ARROW))
	$(call PRINT_MSG,$(BLUE),INFO: $(1))
	$(call PRINT_MSG,$(BLUE),$(UP_ARROW)********************************************************************$(UP_ARROW))
endef
define SUCCESS_MSG
	$(call PRINT_MSG,$(GREEN),**********************************************************************)
	$(call PRINT_MSG,$(GREEN),SUCCESS: $(1))
	$(call PRINT_MSG,$(GREEN),**********************************************************************)
endef
define WARNING_MSG
	$(call PRINT_MSG,$(YELLOW),======================================================================)
	$(call PRINT_MSG,$(YELLOW),WARNING: $(1))
	$(call PRINT_MSG,$(YELLOW),======================================================================)
endef
define ERROR_MSG
	$(call PRINT_MSG,$(RED),======================================================================)
	$(call PRINT_MSG,$(RED),ERROR: $(1) \n)
	$(call PRINT_MSG,$(RED),Build stopped. Please fix error above)
	$(call PRINT_MSG,$(RED),======================================================================)
endef

#=================================================================
# For shell scripts
#=================================================================
DECOR_BLUE_LINE 	= printf "%b\n" "$(BLUE)**********************************************************************$(RESET)"
DECOR_GREEN_LINE	= printf "%b\n" "$(GREEN)**********************************************************************$(RESET)"
DECOR_YELLOW_LINE	= printf "%b\n" "$(YELLOW)**********************************************************************$(RESET)"
DECOR_RED_LINE		= printf "%b\n" "$(RED)**********************************************************************$(RESET)"

WARN_YELLOW_LINE	= printf "%b\n" "$(YELLOW)======================================================================$(RESET)"
ERROR_RED_LINE		= printf "%b\n" "$(RED)======================================================================$(RESET)"

INFO_LINE			= printf "%b\n" "$(BLUE)INFO: $(1)$(RESET)"
SUCCESS_LINE		= printf "%b\n" "$(GREEN)SUCCESS: $(1)$(RESET)"
WARN_LINE			= printf "%b\n" "$(YELLOW)WARNING: $(1)$(RESET)"
ERROR_LINE			= printf "%b\n" "$(RED)ERROR: $(1)$(RESET)"
