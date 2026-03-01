#ifndef CONSTANTS_H
#define CONSTANTS_H

//=================================================================
// Define CONSTANTS
//=================================================================

//=================================================================
// Screen 640 x 480 Default for R36S
//=================================================================
#define SCREEN_WIDTH 					640
#define SCREEN_HEIGHT 					480

//=================================================================
// Modal Screen settings
//=================================================================
#define MODAL_TITLE_MAX					32
#define MODAL_MESSAGE_MAX				256

//=================================================================
// 30 FPS maybe optimal for R36S
//=================================================================
#define TARGET_FRAME_RATE 				60

//=================================================================
// Font Sizes
//=================================================================
#define SMALL_FONT_SIZE 				16
#define DEFAULT_FONT_SIZE 				24
#define LARGE_FONT_SIZE 				32

//=================================================================
// Firing Trigger Treshold
// Analog trigger treshold
//=================================================================
#define FIRING_TRIGGER_TRESHOLD 		0.1f

//=================================================================
// Tumbstick deadzone
//=================================================================
#define TUMBSTICK_DEADZONE_THRESHOLD 	0.2f

//=================================================================
// Thresholds for directional movement
//=================================================================
#define MOVE_VERTICAL_THRESHOLD 		0.5f
#define MOVE_HORIZONTAL_THRESHOLD 		0.5f
#define MOVE_DIAGONAL_THRESHOLD 		0.5f

//=================================================================
// Message Buffer Size
//=================================================================
#define BUFFER_SIZE 					128

//=================================================================
// Controller Overlay Offsets
//=================================================================
#define CONTROLLER_OVERLAY_OFFSET_X 	153

//=================================================================
// IP ADDRESS Lookup
//=================================================================
#define IP_ADDRESS_MAX_RETRIES 			5
#define IP_ADDRESS_MAX_RETRY_INTERVAL 	0.5f
#define IP_ADDRESS_MAX_LEN 				16

//=================================================================
// R32S Device Telemetry
// Tested on device that returned following from:
//
// ark@rg351mp:~$ cat /proc/cpuinfo
//
// CPU implementer : 0x41 		=> ARM Ltd
// CPU architecture: 8 			=> ARMv8-A (64-bit capable ARM)
// CPU part        : 0xd04 		=> ARM Cortex-A35
// Rockchip RK3326 SoC:
// 		Quad-core ARM Cortex-A35
// 		ARM Mali GPU
// See https://rockchip.fr/RK3326%20datasheet%20V1.1.pdf
//=================================================================
#define MAX_CORES						4
#define CPU_SAMPLE_UNINITIALIZED 		(-1e9f)

//=================================================================
// Sound Manager
//=================================================================
//-----------------------------------------------------------------
// SFX: Test limits as SFX file sizes may vary
//-----------------------------------------------------------------
#define MAX_SFX 						16
//-----------------------------------------------------------------

//-----------------------------------------------------------------
// Audio Tracks: Test limits as track file sizes may vary
//-----------------------------------------------------------------
#define MAX_MUSIC_TRACKS				2

//-----------------------------------------------------------------
// Sound Asset Paths
//-----------------------------------------------------------------
#define SOUND_SETTINGS_PATH 			"assets/sound_settings.ini"
#define SFX_CONFIG_PATH 				"assets/audio_sfx.ini"
#define MUSIC_CONFIG_PATH				"assets/audio_tracks.ini"

//-----------------------------------------------------------------
// R36S-specific limits for resource management
//-----------------------------------------------------------------
#if defined(PLATFORM_R36S)
// Max SFX playing at once
#define MAX_CONCURRENT_SFX 				4
// Lazy load by default on R36S
#define DEFAULT_PRELOAD_SFX 			false
// Lazy load music on R36S
#define DEFAULT_PRELOAD_MUSIC 			false
#else
// More on desktop
#define MAX_CONCURRENT_SFX 				8
// Preload on desktop
#define DEFAULT_PRELOAD_SFX 			true
// Lazy load music (large files)
#define DEFAULT_PRELOAD_MUSIC 			false
#endif



//=================================================================
// Spine Animation Configuration
//=================================================================
#define MAX_SPINE_ENTITIES 				64
#define MAX_CACHED_SKELETONS 			16
#define MAX_DEBUG_QUADS 				512

//-----------------------------------------------------------------
// Mesh Pool Configuration (optimised for multiple entities)
//-----------------------------------------------------------------
#define MAX_ATTACHMENTS_PER_SKELETON 	50
#define MAX_CONCURRENT_SKELETONS 		40

//-----------------------------------------------------------------
// MAX_CACHED_MODELS will have to be reduced after testing
//-----------------------------------------------------------------
#define MAX_CACHED_MODELS 				(MAX_ATTACHMENTS_PER_SKELETON * MAX_CONCURRENT_SKELETONS)

//=================================================================
// Memory Management
//=================================================================

//-----------------------------------------------------------------
// Maximum number of pointer allocations that can be tracked
//-----------------------------------------------------------------
#define MAX_MEMORY_ALLOCATIONS			512

//-----------------------------------------------------------------
// Max Name Length for Object being tracked in Memory Manager
//-----------------------------------------------------------------
#define MAX_NAME_LENGTH					82

//=================================================================
// Maximum number of shaders in Shader Manager
//=================================================================
#define MAX_MANAGED_SHADERS 			16

//=================================================================
// Achivements constants including Data files for Achivements Manager
//=================================================================

#define MAX_ACHIEVEMENTS 				24
#define MAX_CATEGORIES 					3
#define MAX_OBSERVERS 					24
#define MAX_LEVELS 						3
#define MAX_ACHIEVEMENT_NAME			32
#define MAX_ACHIEVEMENT_DESC			64
#define MAX_CATEGORY_NAME				32

//-----------------------------------------------------------------
// Achivements display constants
//-----------------------------------------------------------------
// Seconds to show popup
#define ACHIEVEMENT_DISPLAY_DURATION 	3.0f
// Pixels wide
#define ACHIEVEMENT_POPUP_WIDTH 		300
// Pixels tall	  
#define ACHIEVEMENT_POPUP_HEIGHT		60
// Distance from left edge
#define ACHIEVEMENT_POPUP_X_OFFSET		10
// Top of first popup
#define ACHIEVEMENT_POPUP_Y_START		50
// Gap between stacked popups
#define ACHIEVEMENT_POPUP_Y_SPACING 	70

//-----------------------------------------------------------------
// Flag: Achievement applies to all levels
//-----------------------------------------------------------------
#define ALL_LEVELS_ACHIEVEMENT			0

//-----------------------------------------------------------------
// Achievement Progress File
//-----------------------------------------------------------------
#define ACHIEVEMENT_PROGRESS_FILE 		"data/achievement_progress.data"
#define LEVEL_PROGRESS_FILE 			"data/level_progress.data"

//-----------------------------------------------------------------
// Modal Dialog Backgrounds
//-----------------------------------------------------------------
#define SCENE_SPLASH_BACKGROUND_FILE			"./assets/backgrounds/SPLASH_background.png"
#define SCENE_GAMEPLAY_BACKGROUND_FILE 			"./assets/backgrounds/GAMEPLAY_background.png"
#define SCENE_LEVEL_COMPLETE_BACKGROUND_FILE 	"./assets/backgrounds/LEVEL_COMPLETE_background.png"
#define SCENE_GAME_OVER_BACKGROUND_FILE			"./assets/backgrounds/GAME_OVER_background.png"
#define SCENE_GAME_COMPLETE_BACKGROUND_FILE		"./assets/backgrounds/GAME_COMPLETE_background.png"

#endif // CONSTANTS_H