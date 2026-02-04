/*******************************************************************************************
 * R36S Controller Button Tester
 * NOTE: Cross-Platform Input StarterKit
 * Works on Linux | Windows | Web | R36S
 ********************************************************************************************/

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <string.h>

#include "input_manager.h"
#include "command.h"
#include "constants.h"
#include "ip_address.h"

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
#include "telemetry.h"
#endif // Included for R36S and Linux only

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
#if defined(GRAPHICS_API_OPENGL_ES3)
#include <GLES3/gl3.h> // GRAPHICS_API_OPENGL_ES3
#elif defined(GRAPHICS_API_OPENGL_ES2)
#include <GLES2/gl2.h> // GRAPHICS_API_OPENGL_ES2
#else
#include <GL/gl.h>
#endif // Default
#endif // R36S and Linux only

// IP Address
char ip_address_str[IP_ADDRESS_MAX_LEN];
int has_ip_address = 0;
double last_ip_lookup = -1000.0;
int ip_lookups_left = 0;

// Blank Message
char message[BUFFER_SIZE] = "";

// Is there a message
bool has_messages = false;

// Last Message Update
static double last_update = 0;

// Display Message
static char last_message[BUFFER_SIZE] = "";

// Command Bits
char command_bitmap[COMMAND_COUNT + 1];

// Exit Game (Draw Exit and Exit)
static bool exit_game = false;
static double exit_game_time = 0.0;

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
// Telemetry
static Telemetry r36s_telemetry;
static const char *glRendererStr = NULL;
static const char *glVersionStr = NULL;
static const char *glslVersionStr = NULL;
static bool show_telemetry = false;
static bool action_special_2_was_pressed = false;
#endif // TELEMETRY variables R36S and Linux only

// Append Commands
#define APPEND(text)                                           \
	do                                                         \
	{                                                          \
		if (strstr(message, text) == NULL)                     \
		{                                                      \
			size_t len = strlen(message);                      \
			snprintf(message + len, BUFFER_SIZE - len, "%s%s", \
					 has_messages ? "\n" : "", text);          \
			has_messages = true;                               \
		}                                                      \
	} while (0)

typedef struct GameData
{
	// Game Background
	Texture2D background;

	// Player Data
	Vector2 player_position;
	float speed;
	float radius;
	Color player_color;

	// Min Max Game Area
	float min_X;
	float max_X;
	float min_Y;
	float max_Y;

	// Command from Input Manager
	Command command;

	// Time now
	double now;
} GameData;

// Game Data
static GameData game_data = {0};

// Game Function Prototypes
void DrawInputOverlay(Command command);

static void InitGame(void);
static void Input(void);
static void Update(void);
static void Draw(void);
static void Exit(void);

// Mainline
int main(void)
{
	InitGame();

	// Update Loop
	while (!WindowShouldClose())
	{
		Input();
		Update();
		Draw();

		// Check if exiting game (Draws Exit before closing game)
		if (exit_game && (game_data.now - exit_game_time) > 0.50f)
		{
			break;
		}
	}

	Exit();

	return 0;
}

static void InitGame(void)
{
	// R36S window size is 640x480
	InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Cross-Platform Input StarterKit");

	// 30 seems to be a good framerate, set to 60 fps but experiment with 30 if
	// cycle times slow
	SetTargetFPS(TARGET_FRAME_RATE);

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Telemetry Init
	glRendererStr = (const char *)glGetString(GL_RENDERER);
	glVersionStr = (const char *)glGetString(GL_VERSION);
	glslVersionStr = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
	InitTelemetry(&r36s_telemetry);
#endif // Init Telemetry R36S and Linux only

	// Default background
	game_data.background = LoadTexture("./assets/R36SControls480.png");

	// Initialise Input Manager
	InitInputManager();

	// Get IP Address
	has_ip_address = GetIPAddressString(ip_address_str);
	if (!has_ip_address)
		ip_lookups_left = IP_ADDRESS_MAX_RETRIES;
	else
		ip_lookups_left = 0;

	// Set player position and size
	game_data.player_position = (Vector2){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
	game_data.speed = 5.0f;
	game_data.radius = 20.0f;

	// Keep player within screen bounds
	game_data.min_X = CONTROLLER_OVERLAY_OFFSET_X + 45 + game_data.radius;
	game_data.max_X = SCREEN_WIDTH - (CONTROLLER_OVERLAY_OFFSET_X + 50 + game_data.radius);
	game_data.min_Y = 35 + game_data.radius;
	game_data.max_Y = SCREEN_HEIGHT - (game_data.radius + 255);

	// Calculate min max for x and y
	if (game_data.max_X < game_data.min_X)
		game_data.max_X = game_data.min_X;
	if (game_data.max_Y < game_data.min_Y)
		game_data.max_Y = game_data.min_Y;
}

static void Input(void)
{
	// Poll input from Input Manager
	game_data.command = PollInput();

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Toggle telemetry with ACTION_SPECIAL_2
	bool action_special_2_pressed = IsCommandActive(game_data.command, ACTION_SPECIAL_2);

	if (action_special_2_pressed && !action_special_2_was_pressed)
	{
		show_telemetry = !show_telemetry;
		TraceLog(LOG_INFO, "Telemetry display %s", show_telemetry ? "ON" : "OFF");
	}
	action_special_2_was_pressed = action_special_2_pressed;

#endif
}

static void Update(void)
{
	// Reset message each frame
	message[0] = '\0';	  // clear message buffer
	has_messages = false; // reset "first message" flag

	// Get time
	game_data.now = GetTime();

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Telemetry Update
	UpdateTelemetryFrame(&r36s_telemetry, GetFrameTime(), GetFPS());
	UpdateTelemetry(&r36s_telemetry, game_data.now);
#endif // TELEMETRY Update R36S and Linux only

	// Fetch IP Address
	// Only try once per second until we have a non-loopback IP.
	// (If you want it to update forever, remove `!have_ip`.)
	if (!has_ip_address && ip_lookups_left > 0 && (game_data.now - last_ip_lookup) > IP_ADDRESS_MAX_RETRY_INTERVAL)
	{
		last_ip_lookup = game_data.now;
		ip_lookups_left--;
		has_ip_address = GetIPAddressString(ip_address_str);
	}

	// Sample commands to keep main file small and understable

	// Exit Game
	if (IsCommandActive(game_data.command, EXIT_COMMAND) && !exit_game)
	{
		APPEND("Exit");
		exit_game = true;
		exit_game_time = game_data.now;
	}

	// Save Game
	if (IsCommandActive(game_data.command, POWER_COMMAND))
	{
		APPEND("Save or Restore Game");
	}

	// Start Game
	if (IsCommandActive(game_data.command, START_GAME))
	{
		APPEND("Start Game");
	}

	// Menu Game
	if (IsCommandActive(game_data.command, MENU_TOGGLE))
	{
		APPEND("Toggle Menu");
	}

	// Volume Up
	if (IsCommandActive(game_data.command, VOLUME_UP))
	{
		APPEND("Volume ++");
	}

	// Volume Down
	if (IsCommandActive(game_data.command, VOLUME_DOWN))
	{
		APPEND("Volume --");
	}

	// Aim
	if (IsCommandActive(game_data.command, AIM))
	{
		APPEND("Aiming");
	}

	// ACTION_JUMP
	if (IsCommandActive(game_data.command, ACTION_JUMP))
	{
		APPEND("ACTION_JUMP");
	}

	// ACTION_CROUCH
	if (IsCommandActive(game_data.command, ACTION_CROUCH))
	{
		APPEND("ACTION_CROUCH");
	}

	// ACTION_PICKUP
	if (IsCommandActive(game_data.command, ACTION_PICKUP))
	{
		APPEND("ACTION_PICKUP");
	}

	// ACTION_RUN
	if (IsCommandActive(game_data.command, ACTION_RUN))
	{
		APPEND("ACTION_RUN");
	}

	// Process commands

	// Move Player
	if (IsCommandActive(game_data.command, MOVE_UP))
	{
		APPEND("MOVE_UP");
		game_data.player_position.y -= game_data.speed;
	}
	if (IsCommandActive(game_data.command, MOVE_DOWN))
	{
		APPEND("MOVE_DOWN");
		game_data.player_position.y += game_data.speed;
	}
	if (IsCommandActive(game_data.command, MOVE_LEFT))
	{
		APPEND("MOVE_LEFT");
		game_data.player_position.x -= game_data.speed;
	}
	if (IsCommandActive(game_data.command, MOVE_RIGHT))
	{
		APPEND("MOVE_RIGHT");
		game_data.player_position.x += game_data.speed;
	}

	// Clamp player position within Screen Bounds
	game_data.player_position.x = Clamp(game_data.player_position.x, game_data.min_X, game_data.max_X);
	game_data.player_position.y = Clamp(game_data.player_position.y, game_data.min_Y, game_data.max_Y);

	// Color player based on action
	game_data.player_color = DARKGRAY;

	// R36S R1 | Xbox RT
	if (IsCommandActive(game_data.command, ATTACK_PRIMARY))
	{
		APPEND("ATTACK_PRIMARY");
		game_data.player_color = RED;
	}
	// R36S R2 | Xbox RB
	if (IsCommandActive(game_data.command, ATTACK_SECONDARY))
	{
		APPEND("ATTACK_SECONDARY");
		game_data.player_color = DARKPURPLE;
	}
	// R36S L1 | Xbox LT
	if (IsCommandActive(game_data.command, ACTION_SPECIAL_1))
	{
		APPEND("ACTION_SPECIAL_1");
		game_data.player_color = DARKBLUE;
	}
	// R36S L2 | Xbox LB
	if (IsCommandActive(game_data.command, ACTION_SPECIAL_2))
	{
		APPEND("ACTION_SPECIAL_2");
		game_data.player_color = DARKBLUE;
	}
	// R36S L3 Left Tumbstick Button | Xbox Left Tumbstick Button
	if (IsCommandActive(game_data.command, ACTION_SPECIAL_3))
	{
		APPEND("ACTION_SPECIAL_3");
		game_data.player_color = DARKBLUE;
	}
	// R36S R3 Right Tumbstick Button | Xbox Right Tumbstick Button
	if (IsCommandActive(game_data.command, ACTION_SPECIAL_4))
	{
		APPEND("ACTION_SPECIAL_4");
		game_data.player_color = DARKBLUE;
	}

	// No input messages
	if (!has_messages)
	{
		APPEND("NONE");
	}

	// Populate display message
	if (strcmp(message, last_message) != 0 || game_data.now - last_update > 0.2)
	{
		strcpy(last_message, message);
		last_update = game_data.now;
	}

	// Get command bitmap
	GetCommandBits(game_data.command, command_bitmap);
}

static void Draw(void)
{
	// Draw
	BeginDrawing();

	// Clear background
	ClearBackground(GRAY);

	// Draw background picture
	if (game_data.background.id != 0)
	{
		const int x = (SCREEN_WIDTH - game_data.background.width) / 2;
		const int y = (SCREEN_HEIGHT - game_data.background.height) / 2;
		DrawTexture(game_data.background, x, y, WHITE);
	}

	// Draw heading
	DrawText("R36S | Xbox Input", CONTROLLER_OVERLAY_OFFSET_X + 70, 50, SMALL_FONT_SIZE, DARKGRAY);

	// Draw Platform
#if defined(PLATFORM_R36S)
	DrawText("Platform: R36S", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
	DrawText("Press FN to exit", CONTROLLER_OVERLAY_OFFSET_X + 70, 90, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_LINUX)
	DrawText("Platform: Linux", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_WINDOWS)
	DrawText("Platform: Windows", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#elif defined(PLATFORM_WEB)
	DrawText("Platform: Web", CONTROLLER_OVERLAY_OFFSET_X + 70, 70, SMALL_FONT_SIZE, DARKGRAY);
#endif

	// Draw IP Address
	DrawText(TextFormat("%s", ip_address_str), CONTROLLER_OVERLAY_OFFSET_X + 70, 110, LARGE_FONT_SIZE, DARKGRAY);

	// Draw input message
	DrawText(last_message, CONTROLLER_OVERLAY_OFFSET_X + 70, 150, SMALL_FONT_SIZE, WHITE);

	// Show tumbstick values (use input_manager not raw here, this is for demo only)
	float stick_Left_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
	float stick_Left_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

	float stick_Right_X = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_X);
	float stick_Right_Y = GetGamepadAxisMovement(0, GAMEPAD_AXIS_RIGHT_Y);

	// Draw tumbstick values (this is for demo only)
	DrawText(TextFormat("X:%.2f Y:%.2f", stick_Left_X, stick_Left_Y), CONTROLLER_OVERLAY_OFFSET_X + 40, 420, SMALL_FONT_SIZE, DARKGRAY);
	DrawText(TextFormat("X:%.2f Y:%.2f", stick_Right_X, stick_Right_Y), CONTROLLER_OVERLAY_OFFSET_X + 190, 420, SMALL_FONT_SIZE, DARKGRAY);

	// Draw command bitmap (this is for demo only)
	DrawText(command_bitmap, CONTROLLER_OVERLAY_OFFSET_X + 75, 220 - 20, SMALL_FONT_SIZE, DARKGRAY);

	// Draw Player (over everything)
	DrawCircleV(game_data.player_position, game_data.radius, game_data.player_color);

	// Draw Buttons
	DrawInputOverlay(game_data.command);

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
	// Draw Telemetry
	if (show_telemetry)
	{
		DrawTelemetry(&r36s_telemetry, 8, 8, glRendererStr, glVersionStr, glslVersionStr);
	}
#endif // Draw Telemetry R36S and Linux only

	EndDrawing();
}

static void Exit(void)
{
	// Cleanup
	ExitInputManager();
	UnloadTexture(game_data.background);
	CloseWindow();
}

// Function to draw R36S Controller overlay
void DrawInputOverlay(Command command)
{

	// Pulse animation
	float pulse = (sinf(GetTime() * 6.0f) + 1.0f) * 0.5f;

	// D-pad
	if (IsCommandActive(command, MOVE_UP))
	{
		// D-Pad Up
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 87, 260, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_DOWN))
	{
		// D-Pad Down
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 87, 310, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_LEFT))
	{
		// D-Pad Left
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 62, 285, 10, pulseColor);
	}
	if (IsCommandActive(command, MOVE_RIGHT))
	{
		// D-Pad Right
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 110, 285, 10, pulseColor);
	}

	// Thumbsticks
	if (IsCommandActive(command, AIM) || IsCommandActive(command, ACTION_SPECIAL_4))
	{
		// Right Thumbstick (R3)
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 245, 375, 20, pulseColor);
	}
	if (IsCommandActive(command, MOVE_UP) || IsCommandActive(command, MOVE_DOWN) ||
		IsCommandActive(command, MOVE_LEFT) || IsCommandActive(command, MOVE_RIGHT) ||
		IsCommandActive(command, ACTION_SPECIAL_3))
	{
		// Left Thumbstick (L3)
		Color pulseColor = Fade(DARKGRAY, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 85, 375, 20, pulseColor);
	}

	// Trigger buttons (L1, L2, R1, R2)
	if (IsCommandActive(command, ACTION_SPECIAL_1))
	{
		// L1 (R36S) / LT (Xbox) - Left trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 50, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ACTION_SPECIAL_2))
	{
		// L2 (R36S) / LB (Xbox) - Left bumper trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 110, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ATTACK_PRIMARY))
	{
		// R1 (R36S) / RT (Xbox) - Right trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 240, 225, 50, 70, pulseColor);
	}
	if (IsCommandActive(command, ATTACK_SECONDARY))
	{
		// R2 (R36S) / RB (Xbox) - Right bumper trigger
		Color pulseColor = Fade(DARKGRAY, 0.3f + 0.5f * pulse);
		DrawRectangle(CONTROLLER_OVERLAY_OFFSET_X + 190, 225, 50, 70, pulseColor);
	}

	// Face buttons
#if defined(PLATFORM_R36S)
	// R36S face buttons: Top = X (Blue), Bottom = B (Yellow), Left = Y (Green), Right = A (Red)
	if (IsCommandActive(command, ACTION_PICKUP))
	{
		// X (Top - Blue)
		Color pulseColor = Fade(DARKBLUE, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 260, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_CROUCH))
	{
		// B (Bottom - Yellow)
		Color pulseColor = Fade(YELLOW, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 310, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_RUN))
	{
		// Y (Top - Green)
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 220, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_JUMP))
	{
		// A (Right - Red)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 280, 285, 12, pulseColor);
	}
#else
	// Xbox face buttons: Top = Y (Yellow), Bottom = A (Green), Left = X (Blue), Right = B (Red)
	if (IsCommandActive(command, ACTION_JUMP))
	{
		// A (Bottom - Green)
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 310, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_CROUCH))
	{
		// B (Right - Red)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 280, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_PICKUP))
	{
		// X (Left - Blue)
		Color pulseColor = Fade(DARKBLUE, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 220, 285, 12, pulseColor);
	}
	if (IsCommandActive(command, ACTION_RUN))
	{
		// Y (Top - Yellow)
		Color pulseColor = Fade(YELLOW, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 250, 260, 12, pulseColor);
	}
#endif

	// Center buttons
#if defined(PLATFORM_R36S)
	if (IsCommandActive(command, EXIT_COMMAND))
	{
		// FN (Exits Game)
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 165, 310, 12, pulseColor);
	}
#endif // Centre Button R36S

	if (IsCommandActive(command, START_GAME))
	{
		// START
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 195, 340, 12, pulseColor);
	}
	if (IsCommandActive(command, MENU_TOGGLE))
	{
		// SELECT
		Color pulseColor = Fade(DARKGREEN, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 135, 340, 12, pulseColor);
	}

	// Side buttons
#if defined(PLATFORM_R36S)
	if (IsCommandActive(command, POWER_COMMAND))
	{
		// Power
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 10, 120, 12, pulseColor);
	}
	if (IsCommandActive(command, VOLUME_UP))
	{
		// Vol+
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 320, 120, 12, pulseColor);
	}
	if (IsCommandActive(command, VOLUME_DOWN))
	{
		// Vol-
		Color pulseColor = Fade(RED, 0.4f + 0.6f * pulse);
		DrawCircle(CONTROLLER_OVERLAY_OFFSET_X + 320, 160, 12, pulseColor);
	}
#endif
}