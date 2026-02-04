# ----------------------------------------
# Build target for web (emscripten)
# ----------------------------------------
.PHONY: __debug_web __release_web __kill_port_8080

__kill_port_8080:
	$(call INFO_MSG,Freeing port 8080 $(TARGET) (if in use))
	$(call INFO_MSG,Make sure no critical services are using this port)
	$(call INFO_MSG,Verify via ip address if running on server)
	@fuser -k 8080/tcp 2>/dev/null || true

__debug_web: __kill_port_8080 __validate_web_platform __validate_debug_config $(TARGET)
	$(call INFO_MSG,Starting Debug $(TARGET) for Web)
	cp $(WEB_DIR)/icon/favicon.ico $(BUILD_DIR)/favicon.ico
	$(call INFO_MSG,$(MSG_WEB_CLICK_LINK_BELOW))
	$(call INFO_MSG_BOX,http://localhost:8080/$(TARGET_NAME)$(TARGET_EXT))
	$(call INFO_MSG,$(MSG_WEB_CLICK_LINK_ABOVE))
	$(EMSCRIPTEN)/emrun --no_browser --port 8080 ./$(TARGET)$(TARGET_EXT)

__release_web: __kill_port_8080 __validate_web_platform __validate_release_config $(TARGET)
	$(call INFO_MSG,Starting Release $(TARGET) for Web)
	$(call INFO_MSG,$(MSG_WEB_CLICK_LINK_BELOW))
	$(call INFO_MSG_BOX,http://localhost:8080/$(TARGET_NAME)$(TARGET_EXT))
	$(call INFO_MSG,$(MSG_WEB_CLICK_LINK_ABOVE))
	$(EMSCRIPTEN)/emrun --no_browser --port 8080 ./$(TARGET)$(TARGET_EXT)