//=================================================================
// Modal
// A timed full screen overlay shown between game states.
//
// String memory is tracked by name in GlobalMemoryManager so any
// unreleased title/message will surface in MemoryPrintAllocations
// at program exit - no silent leaks.
//=================================================================

#include <string.h>
#include <math.h>
#include <stdio.h>

#include <raylib.h>

#include "game/game_modal.h"
#include "memory/memory_manager.h"
#include "constants.h"

//=================================================================
// Modal Functions
//=================================================================
void ModalInit(Modal *modal)
{
	if (!modal)
		return;

	modal->active = false;
	modal->timer = 0.0f;
	modal->duration = 0.0f;
	modal->title = NULL;
	modal->message = NULL;
	modal->modal_background = NULL;
}

void ModalCreate(Modal *modal,
				 const char *title,
				 const char *message,
				 float duration,
				 Texture2D *modal_background)
{
	if (!modal)
		return;

	// Free any previous strings before overwriting
	ModalCleanup(modal);

	// Allocate and copy title
	size_t title_len = strlen(title ? title : "") + 1;
	modal->title = (char *)MemoryAlloc(title_len, "modal_title");
	if (modal->title)
		memcpy(modal->title, title ? title : "", title_len);

	// Allocate and copy message
	size_t msg_len = strlen(message ? message : "") + 1;
	modal->message = (char *)MemoryAlloc(msg_len, "modal_message");
	if (modal->message)
		memcpy(modal->message, message ? message : "", msg_len);

	// Borrow modal_background - caller retains ownership
	modal->modal_background = modal_background;

	modal->duration = duration;
	modal->timer = 0.0f;
	modal->active = true;
}

void ModalCleanup(Modal *modal)
{
	if (!modal)
		return;

	if (modal->title)
	{
		MemoryFree(modal->title);
		modal->title = NULL;
	}

	if (modal->message)
	{
		MemoryFree(modal->message);
		modal->message = NULL;
	}

	// modal_background is NOT unloaded - caller owns it
	modal->modal_background = NULL;
	modal->active = false;
}

//=================================================================
// Control
//=================================================================
void ModalActivate(Modal *modal)
{
	if (!modal)
		return;
	modal->active = true;
	modal->timer = 0.0f;
}

void ModalDeactivate(Modal *modal)
{
	if (!modal)
		return;
	modal->active = false;
}

bool ModalIsActive(const Modal *modal)
{
	return modal && modal->active;
}

//=================================================================
// Update
//=================================================================
void ModalUpdate(Modal *modal, float dt)
{
	if (!modal || !modal->active)
		return;

	// duration == 0 means no timer - modal stays open until
	// explicitly dismissed (e.g. GAME_COMPLETE_SCENE)
	if (modal->duration <= 0.0f)
		return;

	modal->timer += dt;
	if (modal->timer >= modal->duration)
		modal->active = false;
}

//=================================================================
// Draw
//=================================================================
void ModalDraw(const Modal *modal)
{
	if (!modal || !modal->active)
		return;

	// Semi-transparent overlay behind the modal box
	DrawRectangle(0, 0, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT, Fade(BLACK, 0.5f));

	// Modal box dimensions
	const float box_w = SCREEN_WIDTH * 0.75f;
	const float box_h = SCREEN_HEIGHT * 0.75f;
	const float x = (SCREEN_WIDTH - box_w) / 2.0f;
	const float y = (SCREEN_HEIGHT - box_h) / 2.0f;

	// Background: borrowed texture fills the box, or solid fill fallback
	// Drawn AFTER the overlay so it appears at full brightness inside the box
	if (modal->modal_background && modal->modal_background->id != 0)
	{
		DrawTexturePro(
			*modal->modal_background,
			(Rectangle){0, 0, (float)modal->modal_background->width, (float)modal->modal_background->height},
			(Rectangle){x, y, box_w, box_h},
			(Vector2){0, 0}, 0.0f, WHITE);
	}
	else
	{
		DrawRectangle((int)x, (int)y, (int)box_w, (int)box_h, Fade(LIGHTGRAY, 0.8f));
	}

	// Outline
	DrawRectangleLines((int)x, (int)y, (int)box_w, (int)box_h, DARKGRAY);

	// Title bar (30% of box height)
	const float title_h = box_h * 0.30f;
	DrawRectangle((int)x, (int)y, (int)box_w, (int)title_h, Fade(DARKGRAY, 0.8f));

	// Title and message
	const int padding = 50;
	if (modal->title)
		DrawText(modal->title, (int)x + padding, (int)y + padding, SMALL_FONT_SIZE, WHITE);
	if (modal->message)
		DrawText(modal->message, (int)x + padding, (int)y + padding + 40, SMALL_FONT_SIZE, WHITE);

	// Countdown (top-right of box)
	// Countdown: only shown for timed modals (duration > 0)
	// Countdown: Hidden on GAME_COMPLETE_SCENE which uses duration = 0
	if (modal->duration > 0.0f)
	{
		int countdown = (int)ceilf(modal->duration - modal->timer);
		if (countdown < 0)
			countdown = 0;
		char time_buf[16];
		snprintf(time_buf, sizeof(time_buf), "%d", countdown);
		DrawText(time_buf, (int)(x + box_w) - 120, (int)y + padding, LARGE_FONT_SIZE, WHITE);
	}
}