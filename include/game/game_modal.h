//=================================================================
// Modal Dialog (GAMEPLAY_SCENE still visable but not active)
// A timed full-screen overlay shown between game states.
//
// Texture ownership:
//	Modal does NOT own the modal_background texture.
//	Pass a pointer to a Texture2D owned by the caller (e.g.
//	game_data). Pass NULL for a solid-colour modal_background.
//	This avoids Raylib texture leaks that the caller unloads.
//
// String memory:
//	Title and message are heap-allocated via MemoryAlloc and
//	freed in ModalCleanup(). The memory manager tracks these
//	by name so any leak surfaces at program exit.
//=================================================================

#ifndef MODAL_H
#define MODAL_H

#include <raylib.h>
#include <stdbool.h>

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Modal
	//=================================================================
	typedef struct Modal
	{
		bool active;
		float timer;
		float duration;
		char *title;					// Heap: owned, freed in ModalCleanup
		char *message;					// Heap: owned, freed in ModalCleanup
		Texture2D *modal_background;	// Borrowed: caller owns, never unloaded here
	} Modal;

	//=================================================================
	// Modal Functions
	//=================================================================

	// Initialise a Modal to inactive/empty. Call before first use.
	void ModalInit(Modal *modal);

	// Populate and immediately activate a Modal.
	// modal_background may be NULL (draws solid fill instead).
	// title and message are copied into MemoryAlloc'd strings.
	void ModalCreate(Modal *modal,
					 const char *title,
					 const char *message,
					 float duration,
					 Texture2D *modal_background);

	// Free title/message strings. Safe to call multiple times.
	// Does NOT unload modal_background, that is caller responsibility.
	void ModalCleanup(Modal *modal);

	//=================================================================
	// Control
	//=================================================================
	void ModalActivate(Modal *modal);
	void ModalDeactivate(Modal *modal);
	bool ModalIsActive(const Modal *modal);

	//=================================================================
	// Update / Draw
	//=================================================================
	void ModalUpdate(Modal *modal, float dt);
	void ModalDraw(const Modal *modal);

#ifdef __cplusplus
}
#endif

#endif // MODAL_H