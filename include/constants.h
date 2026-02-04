#ifndef CONTSTANTS_H
#define CONTSTANTS_H

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480

#define TARGET_FRAME_RATE 60

#define SMALL_FONT_SIZE 16
#define DEFAULT_FONT_SIZE 24
#define LARGE_FONT_SIZE 32

// Can be possibly removed
#define RAYLIB_GAMEPAD_AXIS_MAX 6

// Firing Trigger Treshold
#define FIRING_TRIGGER_TRESHOLD 0.1f

// Tumbstick deadzone
#define TUMBSTICK_DEADZONE_THRESHOLD 0.2f

// Thresholds for directional movement
#define MOVE_VERTICAL_THRESHOLD 0.5f
#define MOVE_HORIZONTAL_THRESHOLD 0.5f
#define MOVE_DIAGONAL_THRESHOLD 0.5f

// Message Buffer Size
#define BUFFER_SIZE 128

// Controller Overlay Offsets
#define CONTROLLER_OVERLAY_OFFSET_X 153

// IP ADDRESS
#define IP_ADDRESS_MAX_RETRIES 5
#define IP_ADDRESS_MAX_RETRY_INTERVAL 0.5f
#define IP_ADDRESS_MAX_LEN 16

// R32S Device Telemetry
// Tested on device that returned following from ark@rg351mp:~$ cat /proc/cpuinfo
// CPU implementer : 0x41 		=> ARM Ltd
// CPU architecture: 8 			=> ARMv8-A (64-bit capable ARM)
// CPU part        : 0xd04 		=> ARM Cortex-A35
// Rockchip RK3326 SoC:
// 		Quad-core ARM Cortex-A35
// 		ARM Mali GPU
// See https://rockchip.fr/RK3326%20datasheet%20V1.1.pdf
#define MAX_CORES 4

#endif // CONTSTANTS_H