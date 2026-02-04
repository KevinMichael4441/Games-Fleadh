
# ----------------------------------------
# Debug target for linux host
# This should be run inside the Docker that has been launched with
# GUI support
#
# ***************************
# Run Docker with GUI support
# ***************************
#
# Enter the following commands in the host terminal (not in Docker):
# xhost +local:docker
# sudo docker run -it --rm -v ~/Projects/r36s:/gpp -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix --device /dev/dri r36s-build
#
# Vary the above command based on your local setup
# You can still use debug_linux from inside the container
# without a GUI but you won't see game window just terminal output
# If you want to see game window you need to run with the
# with the docker run command as shown above
# ----------------------------------------

.PHONY: __debug_linux __release_linux

__debug_linux: __validate_linux_platform __validate_debug_config $(TARGET)
	$(call INFO_MSG,Starting Debug $(TARGET) on Linux)
	gdb -x .gdbinit_linux ./$(TARGET)

__release_linux: __validate_linux_platform __validate_release_config $(TARGET)
	$(call INFO_MSG,Starting Release $(TARGET) on Linux)
	./$(TARGET)