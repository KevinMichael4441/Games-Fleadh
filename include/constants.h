#ifndef CONSTANTS_H
#define CONSTANTS_H

//=================================================================
// Screen dimensions
//=================================================================
#define SCREEN_WIDTH    	640
#define SCREEN_HEIGHT   	480

//=================================================================
// FPS Target
//=================================================================
#define TARGET_FPS      	30

//=================================================================
// For OOZE
//=================================================================
#define MAX_POINTS 			7
#define MAX_SPRINGS 		21 	// (6 + 5 + 4 + 3 + 2 + 1)
#define SPRING_CONSTANT		0.1
#define DAMP				0.9
#define BASE_RADIUS			12
#define OOZE_SPEED			0.8f		// 0.8
#define JUMP_AMOUNT			2.5f		// 1.2
#define GRAVITY				0.8f

#define ITERATIONS			6

//=================================================================
// For SecuritySystem
//=================================================================
#define MAX_LASERWALL		1
#define MAX_CAMERA			99
#define MAX_WORKER			1

#define WALL_PUSHBACK		50
#define BOT_PUSHBACK		20

//=================================================================
// Firing Trigger Treshold
// Analog trigger treshold
//=================================================================
#define FIRING_TRIGGER_TRESHOLD 0.1f

//=================================================================
// Tumbstick deadzone
//=================================================================
#define TUMBSTICK_DEADZONE_THRESHOLD 0.2f

//=================================================================
// Thresholds for directional movement
//=================================================================
#define MOVE_VERTICAL_THRESHOLD 0.5f
#define MOVE_HORIZONTAL_THRESHOLD 0.5f
#define MOVE_DIAGONAL_THRESHOLD 0.5f


//=================================================================
// Font Sizes
//=================================================================
#define SMALL_FONT_SIZE 16
#define DEFAULT_FONT_SIZE 24
#define LARGE_FONT_SIZE 32


//=================================================================
// IP ADDRESS Lookup
//=================================================================
#define IP_ADDRESS_MAX_RETRIES 5
#define IP_ADDRESS_MAX_RETRY_INTERVAL 0.5f
#define IP_ADDRESS_MAX_LEN 16

//=================================================================
// R32S Device Telemetry
// Tested on device that returned following from ark@rg351mp:~$ cat /proc/cpuinfo
// CPU implementer : 0x41 		=> ARM Ltd
// CPU architecture: 8 			=> ARMv8-A (64-bit capable ARM)
// CPU part        : 0xd04 		=> ARM Cortex-A35
// Rockchip RK3326 SoC:
// 		Quad-core ARM Cortex-A35
// 		ARM Mali GPU
// See https://rockchip.fr/RK3326%20datasheet%20V1.1.pdf
//=================================================================
#define MAX_CORES 4
#define CPU_SAMPLE_UNINITIALIZED (-1e9f)

#endif // CONSTANTS_H