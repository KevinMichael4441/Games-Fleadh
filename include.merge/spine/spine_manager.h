//=================================================================
// Spine Manager
//=================================================================
#ifndef SPINE_MANAGER_H
#define SPINE_MANAGER_H

#include <raylib.h>
#include <stdbool.h>
#include <spine/spine.h>

#include "constants.h"

//=================================================================
// C linkage (C++ compiler flag)
//=================================================================
#ifdef __cplusplus
extern "C"
{
#endif

	//=================================================================
	// Spine Entity (Forward Declaration)
	//=================================================================
	typedef struct SpineEntity SpineEntity;

	//=================================================================
	// Animation Play Mode
	//=================================================================
	typedef enum
	{
		SPINE_ANIM_ONCE,	  // Play once and stop
		SPINE_ANIM_LOOP,	  // Loop continuously
		SPINE_ANIM_PING_PONG, // Play forward then backward (if supported)
		SPINE_PLAY_MODE_COUNT // Count of play modes
	} SpinePlayMode;

	//=================================================================
	// Spine Manager Stats (debugging/monitoring)
	//=================================================================
	typedef struct
	{
		int entities_active;	  // Currently active entities
		int entities_created;	  // Total entities created (lifetime)
		int entities_destroyed;	  // Total entities destroyed (lifetime)
		int atlas_loaded;		  // Unique atlases loaded
		int skeletons_loaded;	  // Unique skeleton data loaded
		int pool_slots_used;	  // Current mesh pool usage
		int pool_slots_total;	  // Total mesh pool capacity
		int animations_playing;	  // Active animations
		float pool_usage_percent; // Pool usage percentage
	} SpineManagerStats;

//=================================================================
// Debug Drawing State
//=================================================================
#if defined(DEBUG)
	typedef struct
	{
		Vector2 vertices[4];
		Color color;
	} DebugQuad;
#endif

	//=================================================================
	// Init Spine Manager
	// Must be called before any other spine operations
	//=================================================================
	void InitSpineManager();

	//=================================================================
	// Load Spine Config (key=value format)
	// Preloads common spine assets from config file
	// Format: entity_name=atlas_path,json_path
	// Example: player=player.atlas,player.json
	//=================================================================
	bool LoadSpineConfig(const char *filepath, bool preload);

	//=================================================================
	// Set Spine Assets Path (optional, default: "assets/animation/")
	//=================================================================
	void SetSpineAssetsPath(const char *path);

	//=================================================================
	// Create Spine Entity
	// Returns: Pointer to entity, or NULL on failure
	//=================================================================
	SpineEntity *CreateSpineEntity(const char *atlas_file, const char *json_file,
								   float x, float y, float scale);

	//=================================================================
	// Create Spine Entity from Preloaded Data
	// Uses cached atlas/skeleton data (faster)
	// Returns: Pointer to entity, or NULL on failure
	//=================================================================
	SpineEntity *CreateSpineEntityFromCache(const char *entity_name,
											float x, float y, float scale);

	//=================================================================
	// Destroy Spine Entity
	// Frees entity and removes from manager
	//=================================================================
	void DestroySpineEntity(SpineEntity *entity);

	//=================================================================
	// Play Animation by Name
	// track: Animation track (0 for main, 1+ for layering)
	// name: Animation name (e.g., "run", "jump", "idle")
	// mode: SPINE_ANIM_ONCE or SPINE_ANIM_LOOP
	// Returns: true if animation exists and was set
	//=================================================================
	bool PlaySpineAnimation(SpineEntity *entity, int track, const char *name, SpinePlayMode mode);

	//=================================================================
	// Queue Animation (will play after current animation finishes)
	// delay: Seconds to wait before starting (0.0 for immediate)
	// Returns: true if animation exists and was queued
	//=================================================================
	bool QueueSpineAnimation(SpineEntity *entity, int track, const char *name,
							 SpinePlayMode mode, float delay);

	//=================================================================
	// Set Animation Mix Duration (crossfade/blend time)
	// from_name: Source animation name (NULL for any)
	// to_name: Target animation name (NULL for any)
	// duration: Blend time in seconds
	//=================================================================
	void SetSpineAnimationMix(SpineEntity *entity, const char *from_name,
							  const char *to_name, float duration);

	//=================================================================
	// Get Current Animation Name
	// track: Animation track (0 for main)
	// Returns: Animation name, or NULL if none playing
	//=================================================================
	const char *GetSpineCurrentAnimation(SpineEntity *entity, int track);

	//=================================================================
	// Check if Animation is Playing
	// track: Animation track (0 for main)
	// Returns: true if animation is active
	//=================================================================
	bool IsSpineAnimationPlaying(SpineEntity *entity, int track);

	//=================================================================
	// Set Animation Time Scale (speed multiplier)
	// scale: 1.0 = normal, 2.0 = double speed, 0.5 = half speed
	//=================================================================
	void SetSpineAnimationSpeed(SpineEntity *entity, float speed);

	//=================================================================
	// Set Entity Position
	//=================================================================
	void SetSpinePosition(SpineEntity *entity, float x, float y);

	//=================================================================
	// Get Entity Position
	//=================================================================
	void GetSpinePosition(SpineEntity *entity, float *x, float *y);

	//=================================================================
	// Set Entity Scale
	//=================================================================
	void SetSpineScale(SpineEntity *entity, float scale);

	//=================================================================
	// Get Entity Scale
	//=================================================================
	float GetSpineScale(SpineEntity *entity);

	//=================================================================
	// Set Entity Flip (mirror)
	// flip_x: Mirror horizontally
	// flip_y: Mirror vertically
	//=================================================================
	void SetSpineFlip(SpineEntity *entity, bool flip_x, bool flip_y);

	//=================================================================
	// Get Entity Flip State
	//=================================================================
	void GetSpineFlip(SpineEntity *entity, bool *flip_x, bool *flip_y);

	//=================================================================
	// Set Entity Visibility
	//=================================================================
	void SetSpineVisible(SpineEntity *entity, bool visible);

	//=================================================================
	// Get Entity Visibility
	//=================================================================
	bool IsSpineVisible(SpineEntity *entity);

	//=================================================================
	// Get Bounding Box (Axis-Aligned)
	// Returns: Rectangle with entity bounds (world space)
	// Use for collision detection, culling, etc.
	//=================================================================
	Rectangle GetSpineBounds(SpineEntity *entity);

	//=================================================================
	// Get Skeleton Bounds (tighter fit, from Spine data)
	// Returns: Rectangle based on skeleton's computed bounds
	//=================================================================
	Rectangle GetSpineSkeletonBounds(SpineEntity *entity);

	//=================================================================
	// Get Width and Height
	//=================================================================
	float GetSpineWidth(SpineEntity *entity);
	float GetSpineHeight(SpineEntity *entity);

	//=================================================================
	// Check Collision Between Entities
	// Returns: true if bounding boxes overlap
	//=================================================================
	bool CheckSpineCollision(SpineEntity *entity_lhs, SpineEntity *entity_rhs);

	//=================================================================
	// Check Collision with Rectangle
	// Returns: true if entity bounds overlap with rect
	//=================================================================
	bool CheckSpineCollisionRect(SpineEntity *entity, Rectangle rect);

	//=================================================================
	// Update Single Entity
	// delta_time: Delta time in seconds
	//=================================================================
	void UpdateSpineEntity(SpineEntity *entity, float delta_time);

	//=================================================================
	// Update All Entities
	// delta_time: Delta time in seconds
	// Call once per frame to update all active entities
	//=================================================================
	void UpdateAllSpineEntities(float delta_time);

	//=================================================================
	// Draw Single Entity
	//=================================================================
	void DrawSpineEntity(SpineEntity *entity);

	//=================================================================
	// Draw All Entities
	// Call once per frame to render all visible entities
	//=================================================================
	void DrawAllSpineEntities();

	//=================================================================
	// Get Entity Count (active)
	//=================================================================
	int GetSpineEntityCount();

	//=================================================================
	// Get Spine Manager Stats (for monitoring/debugging)
	//=================================================================
	SpineManagerStats GetSpineManagerStats();

	//=================================================================
	// Save Spine Settings (positions, scales, etc.)
	//=================================================================
	bool SaveSpineSettings(const char *filepath);

	//=================================================================
	// Load Spine Settings
	//=================================================================
	bool LoadSpineSettings(const char *filepath);

	//=================================================================
	// Enable/Disable Debug Rendering
	// Shows bounding boxes, skeleton bones, attachment names, etc.
	//=================================================================
	void SetSpineDebugMode(bool enabled);

	//=================================================================
	// Get Debug Mode State
	//=================================================================
	bool IsSpineDebugMode();

	//=================================================================
	// Cleanup Spine Manager
	// Destroys all entities and frees resources
	//=================================================================
	void ExitSpineManager();

#ifdef __cplusplus
}
#endif

#endif // SPINE_MANAGER_H