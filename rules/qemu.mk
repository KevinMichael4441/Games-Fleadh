# ----------------------------------------
# R36S specific targets
# ----------------------------------------
.PHONY: __qemu_test

# ----------------------------------------
# Qemu execution of r36s binary on x64 
# host PC Docker target 
# https://www.qemu.org/docs/master/system/arm/emulation.html
# ----------------------------------------
__qemu_test: __validate_r36s_platform $(TARGET)
	$(call INFO_MSG_HEAD,$(MSG_QEMU_START))
	$(call INFO_MSG,Running $(TARGET) in QEMU)
	@cp -r "$(ASSETS_DIR)" "$(BUILD_DIR)/"
	@qemu-aarch64 -L /usr/aarch64-linux-gnu "./$(TARGET)" >qemu_test.log 2>&1; \
	rc=$$?; \
	if [ $$rc -eq 0 ]; then \
		$(call DECOR_GREEN_LINE); \
		$(call SUCCESS_LINE, $(strip $(MSG_QEMU_END))); \
		$(call DECOR_GREEN_LINE); \
	else \
		$(call ERROR_RED_LINE); \
		$(call ERROR_LINE, $(strip $(MSG_QEMU_ERROR))); \
		$(call ERROR_RED_LINE); \
		tail -n 60 /tmp/qemu_test.log; \
		exit 1; \
	fi