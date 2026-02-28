#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <raylib.h>
#include <rlgl.h>
#include <raymath.h>
#include <spine/spine.h>

#include "spine/spine_manager.h"
#include "constants.h"

//=================================================================
// Cached Model with Mesh Pool
//=================================================================
typedef struct
{
	Mesh mesh;
	Material mat;
	int is_in_use;
} CachedModel;

static CachedModel model_cache[MAX_CACHED_MODELS] = {0};
static int model_cache_slot = 0;

//=================================================================
// Cached Skeleton Data (for CreateSpineEntityFromCache)
//=================================================================
typedef struct
{
	char name[64];
	spAtlas *atlas;
	spSkeletonData *skeleton_data;
	bool in_use;
} CachedSkeletonData;

static CachedSkeletonData cached_skeletons[MAX_CACHED_SKELETONS] = {0};
static int cached_skeleton_count = 0;

//=================================================================
// Spine Entity (Internal Structure)
//=================================================================
struct SpineEntity
{
	// Spine data
	spSkeleton *skeleton;
	spAnimationState *state;
	spAnimationStateData *state_data;
	spAtlas *atlas;
	spSkeletonData *skeleton_data;

	// Transform
	float x;
	float y;
	float scale;
	bool flip_x;
	bool flip_y;

	// State
	bool visible;
	bool owns_atlas;		 // True if entity owns atlas (must free on destroy)
	bool owns_skeleton_data; // True if entity owns skeleton data (must free on destroy)

	// Metadata
	int entity_id;
	bool active;
};

//=================================================================
// Manager State
//=================================================================
static SpineEntity entities[MAX_SPINE_ENTITIES] = {0};
static int entity_next_id = 0;
static bool manager_initialised = false;
static bool debug_mode = false;

// Manager Stats
static SpineManagerStats manager_stats = {0};

// Assets (need to provide _spUtil_readFile path)
static char spine_assets_path[256] = "assets/animation/";

//=================================================================
// Camera 3D for rendering Mesh
//=================================================================
static Camera3D spine_camera = {0};

//=================================================================
// Debug quad storage
//=================================================================
#if defined(DEBUG)
static DebugQuad debug_quads[MAX_DEBUG_QUADS];
static int debug_quad_count = 0;
#endif

#if defined(DEBUG)
static void AddDebugQuad(Vector2 vertex[4], Color color)
{
    if (debug_quad_count < 512) {
        for (int i = 0; i < 4; i++)
            debug_quads[debug_quad_count].vertices[i] = vertex[i];
        debug_quads[debug_quad_count].color = color;
        debug_quad_count++;
    }
}

static void ResetDebugQuads() { debug_quad_count = 0; }
#endif

//=================================================================
// Forward Declarations
//=================================================================
static void InitializeMeshPool();
static void CleanupMeshPool();
static int PrepareNextCacheSlot();
static void UpdateCachedMesh(int slot, Vector2 vertex[4], const float *uvs, Color tint, Texture2D texture);
static void ResetMeshPoolSlot();
static char *SpineAssetPath(const char *path);
static Vector2 SpineToScreenSpace(float x, float y, float scale, float offset_x, float offset_y);
static void DrawRegionTextured(spSlot *slot, spRegionAttachment *region, spAtlas *atlas,
							   float x, float y, float scale);
static void DrawSkeletonInternal(spSkeleton *skeleton, spAtlas *atlas, float x, float y, float scale);

//=================================================================
// Spine Library Hooks (Do Not Change Method Names)
//=================================================================

void *_spUtil_readFile(const char *path, int *length)
{
	FILE *f = fopen(path, "rb");
	if (!f)
		return NULL;
	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *data = (char *)malloc((size_t)size);
	if (!data)
	{
		fclose(f);
		return NULL;
	}
	size_t read = fread(data, 1, (size_t)size, f);
	fclose(f);
	if (read != (size_t)size)
	{
		free(data);
		return NULL;
	}
	*length = (int)size;
	return data;
}

void _spAtlasPage_createTexture(spAtlasPage *self, const char *path)
{
	char *full = SpineAssetPath(path);
	if (!full)
	{
		self->rendererObject = NULL;
		self->width = 0;
		self->height = 0;
		return;
	}
	Image img = LoadImage(full);
	free(full);
	if (img.data == NULL)
	{
		self->rendererObject = NULL;
		self->width = 0;
		self->height = 0;
		return;
	}
	Texture2D *texture = (Texture2D *)malloc(sizeof(Texture2D));
	if (!texture)
	{
		UnloadImage(img);
		self->rendererObject = NULL;
		self->width = 0;
		self->height = 0;
		return;
	}
	*texture = LoadTextureFromImage(img);
	UnloadImage(img);
	if (texture->id == 0)
	{
		free(texture);
		self->rendererObject = NULL;
		self->width = 0;
		self->height = 0;
		return;
	}
	self->rendererObject = texture;
	self->width = texture->width;
	self->height = texture->height;
}

void _spAtlasPage_disposeTexture(spAtlasPage *self)
{
	if (!self || !self->rendererObject)
		return;
	Texture2D *texture = (Texture2D *)self->rendererObject;
	UnloadTexture(*texture);
	free(texture);
	self->rendererObject = NULL;
}

//=================================================================
// Initialize Mesh Pool
//=================================================================
static void InitializeMeshPool()
{
	TraceLog(LOG_INFO, "SPINE_MANAGER: Initialising mesh pool (%d meshes, ~%.2f MB)",
			 MAX_CACHED_MODELS, (MAX_CACHED_MODELS * 1.0f) / 1024.0f);

	for (int i = 0; i < MAX_CACHED_MODELS; i++)
	{
		Mesh mesh = {0};
		mesh.vertexCount = 4;
		mesh.triangleCount = 2;
		mesh.vertices = (float *)MemAlloc(4 * 3 * sizeof(float));
		mesh.texcoords = (float *)MemAlloc(4 * 2 * sizeof(float));
		mesh.normals = (float *)MemAlloc(4 * 3 * sizeof(float));
		mesh.colors = (unsigned char *)MemAlloc(4 * 4 * sizeof(unsigned char));
		mesh.indices = (unsigned short *)MemAlloc(6 * sizeof(unsigned short));

		for (int v = 0; v < 4; v++)
		{
			mesh.vertices[v * 3 + 0] = 0.0f;
			mesh.vertices[v * 3 + 1] = 0.0f;
			mesh.vertices[v * 3 + 2] = 0.0f;
			mesh.texcoords[v * 2 + 0] = 0.0f;
			mesh.texcoords[v * 2 + 1] = 0.0f;
			mesh.normals[v * 3 + 0] = 0.0f;
			mesh.normals[v * 3 + 1] = 0.0f;
			mesh.normals[v * 3 + 2] = 1.0f;
			mesh.colors[v * 4 + 0] = 255;
			mesh.colors[v * 4 + 1] = 255;
			mesh.colors[v * 4 + 2] = 255;
			mesh.colors[v * 4 + 3] = 255;
		}

		mesh.indices[0] = 0;
		mesh.indices[1] = 1;
		mesh.indices[2] = 2;
		mesh.indices[3] = 2;
		mesh.indices[4] = 3;
		mesh.indices[5] = 0;
		UploadMesh(&mesh, false);
		model_cache[i].mesh = mesh;
		model_cache[i].mat = LoadMaterialDefault();
		model_cache[i].is_in_use = 0;
	}

	manager_stats.pool_slots_total = MAX_CACHED_MODELS;
}

//=================================================================
// Cleanup Mesh Pool
//=================================================================
static void CleanupMeshPool()
{
	for (int i = 0; i < MAX_CACHED_MODELS; i++)
	{
		if (model_cache[i].is_in_use)
		{
			UnloadMesh(model_cache[i].mesh);
			UnloadMaterial(model_cache[i].mat);
			model_cache[i].is_in_use = 0;
		}
	}
}

//=================================================================
// Prepare Next Cache Slot
//=================================================================
static int PrepareNextCacheSlot()
{
	if (model_cache_slot >= MAX_CACHED_MODELS)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Mesh pool exhausted: Used %d/%d slots.",
				 model_cache_slot, MAX_CACHED_MODELS);
		return 0;
	}
	int slot = model_cache_slot;
	model_cache[slot].is_in_use = 1;
	model_cache_slot++;
	return slot;
}

//=================================================================
// Update Cached Mesh
//=================================================================
static void UpdateCachedMesh(int slot, Vector2 vertex[4], const float *uvs, Color tint, Texture2D texture)
{
	Mesh *mesh = &model_cache[slot].mesh;

	mesh->vertices[0] = vertex[0].x;
	mesh->vertices[1] = vertex[0].y;
	mesh->vertices[2] = 0.0f;
	mesh->vertices[3] = vertex[1].x;
	mesh->vertices[4] = vertex[1].y;
	mesh->vertices[5] = 0.0f;
	mesh->vertices[6] = vertex[2].x;
	mesh->vertices[7] = vertex[2].y;
	mesh->vertices[8] = 0.0f;
	mesh->vertices[9] = vertex[3].x;
	mesh->vertices[10] = vertex[3].y;
	mesh->vertices[11] = 0.0f;

	mesh->texcoords[0] = uvs[0];
	mesh->texcoords[1] = uvs[1];
	mesh->texcoords[2] = uvs[2];
	mesh->texcoords[3] = uvs[3];
	mesh->texcoords[4] = uvs[4];
	mesh->texcoords[5] = uvs[5];
	mesh->texcoords[6] = uvs[6];
	mesh->texcoords[7] = uvs[7];

	for (int i = 0; i < 4; i++)
	{
		mesh->colors[i * 4 + 0] = tint.r;
		mesh->colors[i * 4 + 1] = tint.g;
		mesh->colors[i * 4 + 2] = tint.b;
		mesh->colors[i * 4 + 3] = tint.a;
	}

	UpdateMeshBuffer(*mesh, 0, mesh->vertices, 4 * 3 * sizeof(float), 0);
	UpdateMeshBuffer(*mesh, 1, mesh->texcoords, 4 * 2 * sizeof(float), 0);
	UpdateMeshBuffer(*mesh, 3, mesh->colors, 4 * 4 * sizeof(unsigned char), 0);
	model_cache[slot].mat.maps[MATERIAL_MAP_DIFFUSE].texture = texture;
}

//=================================================================
// Reset Mesh Pool Slot (call once per frame)
//=================================================================
static void ResetMeshPoolSlot()
{
	model_cache_slot = 0;
}

//=================================================================
// Set Spine Assets Path (optional, default: "assets/animation/")
//=================================================================
void SetSpineAssetsPath(const char *path)
{
	snprintf(spine_assets_path, sizeof(spine_assets_path), "%s", path);
	TraceLog(LOG_INFO, "SPINE_MANAGER: Assets path set to '%s'", spine_assets_path);
}

//=================================================================
// Spine Asset Path Helper
//=================================================================
static char *SpineAssetPath(const char *path)
{
	if (!path)
		return NULL;

	if (strchr(path, '/') || strchr(path, '\\'))
		return strdup(path);

	size_t n = strlen(spine_assets_path) + strlen(path) + 1;

	char *out = (char *)malloc(n);

	if (!out)
		return NULL;

	snprintf(out, n, "%s%s", spine_assets_path, path);

	return out;
}

//=================================================================
// Spine to Screen Space
//=================================================================
static Vector2 SpineToScreenSpace(float x, float y, float scale, float offset_x, float offset_y)
{
	return (Vector2){offset_x + x * scale, offset_y - y * scale};
}

//=================================================================
// Draw Region Textured (Internal)
//=================================================================
static void DrawRegionTextured(spSlot *slot, spRegionAttachment *region, spAtlas *atlas,
							   float x, float y, float scale)
{
	spAtlasRegion *atlas_region = (spAtlasRegion *)region->rendererObject;

	// Handle Spine 4.x sequences
	if (!atlas_region && region->sequence)
	{
		char region_name[256];
		snprintf(region_name, sizeof(region_name), "%s%02d", region->path, slot->sequenceIndex + 1);
		atlas_region = spAtlas_findRegion(atlas, region_name);
	}

	if (!atlas_region || !atlas_region->page || !atlas_region->page->rendererObject)
		return;
	Texture2D *texture = (Texture2D *)atlas_region->page->rendererObject;
	if (!texture || texture->id == 0)
		return;

	float world[8];
	spRegionAttachment_computeWorldVertices(region, slot, world, 0, 2);

	Vector2 vertex[4];
	vertex[0] = SpineToScreenSpace(world[0], world[1], scale, x, y);
	vertex[1] = SpineToScreenSpace(world[2], world[3], scale, x, y);
	vertex[2] = SpineToScreenSpace(world[4], world[5], scale, x, y);
	vertex[3] = SpineToScreenSpace(world[6], world[7], scale, x, y);

	// Add debug quad capture
    #if defined(DEBUG)
    if (debug_mode) {
        AddDebugQuad(vertex, YELLOW);
    }
    #endif

	spColor *color = &slot->color;
	Color tint = {(unsigned char)(color->r * 255), (unsigned char)(color->g * 255),
				  (unsigned char)(color->b * 255), (unsigned char)(color->a * 255)};

	int cache_slot = PrepareNextCacheSlot();
	UpdateCachedMesh(cache_slot, vertex, region->uvs, tint, *texture);
	DrawMesh(model_cache[cache_slot].mesh, model_cache[cache_slot].mat, MatrixIdentity());
}

//=================================================================
// Draw Skeleton Internal
//=================================================================
static void DrawSkeletonInternal(spSkeleton *skeleton, spAtlas *atlas, float x, float y, float scale)
{
	if (!skeleton)
		return;

	for (int i = 0; i < skeleton->slotsCount; i++)
	{
		spSlot *slot = skeleton->drawOrder[i];
		if (!slot || !slot->attachment)
			continue;
		if (slot->attachment->type == SP_ATTACHMENT_REGION)
			DrawRegionTextured(slot, (spRegionAttachment *)slot->attachment, atlas, x, y, scale);
	}
}

//=================================================================
// Init Spine Manager
//=================================================================
void InitSpineManager()
{
	if (manager_initialised)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Already initialized");
		return;
	}

	TraceLog(LOG_INFO, "SPINE_MANAGER: Initialising");

	// Initialize mesh pool
	InitializeMeshPool();

	// Setup orthographic camera
	spine_camera.position = (Vector3){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, -1000.0f};
	spine_camera.target = (Vector3){SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f, 0.0f};
	spine_camera.up = (Vector3){0.0f, -1.0f, 0.0f};
	spine_camera.fovy = (float)SCREEN_HEIGHT;
	spine_camera.projection = CAMERA_ORTHOGRAPHIC;

	// Initialize entities
	memset(entities, 0, sizeof(entities));
	memset(cached_skeletons, 0, sizeof(cached_skeletons));

	// Reset stats
	memset(&manager_stats, 0, sizeof(manager_stats));
	manager_stats.pool_slots_total = MAX_CACHED_MODELS;

	manager_initialised = true;
	TraceLog(LOG_INFO, "SPINE_MANAGER: Initialisation complete");
}

//=================================================================
// Load Spine Config
//=================================================================
bool LoadSpineConfig(const char *filepath, bool preload)
{
	// TODO: Implement config loading (similar to sound_manager)
	// Format: entity_name=atlas_path,json_path
	TraceLog(LOG_WARNING, "SPINE_MANAGER: LoadSpineConfig not yet implemented");
	return false;
}

//=================================================================
// Find Free Entity Slot
//=================================================================
static SpineEntity *FindFreeEntitySlot()
{
	for (int i = 0; i < MAX_SPINE_ENTITIES; i++)
	{
		if (!entities[i].active)
			return &entities[i];
	}
	return NULL;
}

//=================================================================
// Create Spine Entity
//=================================================================
SpineEntity *CreateSpineEntity(const char *atlas_file, const char *json_file,
							   float x, float y, float scale)
{
	if (!manager_initialised)
	{
		TraceLog(LOG_ERROR, "SPINE_MANAGER: Not initialised, usage InitSpineManager() during Init()");
		return NULL;
	}

	SpineEntity *entity = FindFreeEntitySlot();
	if (!entity)
	{
		TraceLog(LOG_ERROR, "SPINE_MANAGER: Max entities reached (%d)", MAX_SPINE_ENTITIES);
		return NULL;
	}

	char atlas_path[256], json_path[256];
	snprintf(atlas_path, sizeof(atlas_path), "%s%s", spine_assets_path, atlas_file);
	snprintf(json_path, sizeof(json_path), "%s%s", spine_assets_path, json_file);

	// Load atlas
	entity->atlas = spAtlas_createFromFile(atlas_path, NULL);
	if (!entity->atlas)
	{
		TraceLog(LOG_ERROR, "SPINE_MANAGER: Failed to load atlas: %s", atlas_path);
		return NULL;
	}

	// Load skeleton
	spSkeletonJson *json = spSkeletonJson_create(entity->atlas);
	entity->skeleton_data = spSkeletonJson_readSkeletonDataFile(json, json_path);
	if (!entity->skeleton_data)
	{
		TraceLog(LOG_ERROR, "SPINE_MANAGER: Failed to load skeleton: %s", json_path);
		spAtlas_dispose(entity->atlas);
		spSkeletonJson_dispose(json);
		return NULL;
	}

	// Create skeleton instance
	entity->skeleton = spSkeleton_create(entity->skeleton_data);
	spSkeleton_setToSetupPose(entity->skeleton);
	spSkeleton_updateWorldTransform(entity->skeleton, SP_PHYSICS_NONE);

	// Create animation state
	entity->state_data = spAnimationStateData_create(entity->skeleton_data);
	entity->state = spAnimationState_create(entity->state_data);

	spSkeletonJson_dispose(json);

	// Set transform
	entity->x = x;
	entity->y = y;
	entity->flip_x = false;
	entity->flip_y = false;

	// Set state
	entity->visible = true;
	entity->owns_atlas = true;
	entity->owns_skeleton_data = true;
	entity->entity_id = entity_next_id++;
	entity->active = true;

	// Set Scale
	entity->scale = scale;

	// Update stats
	manager_stats.entities_active++;
	manager_stats.entities_created++;

	TraceLog(LOG_INFO, "SPINE_MANAGER: Created entity #%d at (%.0f, %.0f)",
			 entity->entity_id, x, y);

	return entity;
}

//=================================================================
// Create Spine Entity from Cache
//=================================================================
SpineEntity *CreateSpineEntityFromCache(const char *entity_name, float x, float y, float scale)
{
	// TODO: Implement cached entity creation
	TraceLog(LOG_WARNING, "SPINE_MANAGER: CreateSpineEntityFromCache not yet implemented");
	return NULL;
}

//=================================================================
// Destroy Spine Entity
//=================================================================
void DestroySpineEntity(SpineEntity *entity)
{
	if (!entity || !entity->active)
		return;

	// Cleanup Spine data
	if (entity->state)
	{
		spAnimationState_dispose(entity->state);
		entity->state = NULL;
	}

	if (entity->state_data)
	{
		spAnimationStateData_dispose(entity->state_data);
		entity->state_data = NULL;
	}

	if (entity->skeleton)
	{
		spSkeleton_dispose(entity->skeleton);
		entity->skeleton = NULL;
	}

	if (entity->owns_skeleton_data && entity->skeleton_data)
	{
		spSkeletonData_dispose(entity->skeleton_data);
		entity->skeleton_data = NULL;
	}

	if (entity->owns_atlas && entity->atlas)
	{
		spAtlas_dispose(entity->atlas);
		entity->atlas = NULL;
	}

	entity->active = false;

	// Update stats
	manager_stats.entities_active--;
	manager_stats.entities_destroyed++;

	TraceLog(LOG_INFO, "SPINE_MANAGER: Destroyed entity #%d", entity->entity_id);
}

//=================================================================
// Play Animation by Name
//=================================================================
bool PlaySpineAnimation(SpineEntity *entity, int track, const char *name, SpinePlayMode mode)
{
	if (!entity || !entity->active || !entity->state)
		return false;

	spAnimation *anim = spSkeletonData_findAnimation(entity->skeleton_data, name);
	if (!anim)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Animation '%s' not found", name);
		return false;
	}

	bool loop = (mode == SPINE_ANIM_LOOP);
	spAnimationState_setAnimation(entity->state, track, anim, loop ? 1 : 0);

	TraceLog(LOG_INFO, "SPINE_MANAGER: Playing animation '%s' on track %d (loop=%d)",
			 name, track, loop);
	return true;
}

//=================================================================
// Queue Animation
//=================================================================
bool QueueSpineAnimation(SpineEntity *entity, int track, const char *name,
						 SpinePlayMode mode, float delay)
{
	if (!entity || !entity->active || !entity->state)
		return false;

	spAnimation *anim = spSkeletonData_findAnimation(entity->skeleton_data, name);
	if (!anim)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Animation '%s' not found", name);
		return false;
	}

	bool loop = (mode == SPINE_ANIM_LOOP);
	spAnimationState_addAnimation(entity->state, track, anim, loop ? 1 : 0, delay);

	TraceLog(LOG_INFO, "SPINE_MANAGER: Queued animation '%s' on track %d (delay=%.2f)",
			 name, track, delay);
	return true;
}

//=================================================================
// Set Animation Mix Duration
//=================================================================
void SetSpineAnimationMix(SpineEntity *entity, const char *from_name,
						  const char *to_name, float duration)
{
	if (!entity || !entity->active || !entity->state_data)
		return;

	spAnimation *from = from_name ? spSkeletonData_findAnimation(entity->skeleton_data, from_name) : NULL;
	spAnimation *to = to_name ? spSkeletonData_findAnimation(entity->skeleton_data, to_name) : NULL;

	if (from && to)
	{
		spAnimationStateData_setMix(entity->state_data, from, to, duration);
		TraceLog(LOG_INFO, "SPINE_MANAGER: Set mix '%s' -> '%s' = %.2fs",
				 from_name, to_name, duration);
	}
	else if (!from && to)
	{
		// Set default mix to target animation
		spAnimationStateData_setMixByName(entity->state_data, NULL, to_name, duration);
	}
}

//=================================================================
// Get Current Animation Name
//=================================================================
const char *GetSpineCurrentAnimation(SpineEntity *entity, int track)
{
	if (!entity || !entity->active || !entity->state)
		return NULL;
	if (track < 0 || track >= entity->state->tracksCount)
		return NULL;

	spTrackEntry *entry = entity->state->tracks[track];
	return entry && entry->animation ? entry->animation->name : NULL;
}

//=================================================================
// Check if Animation is Playing
//=================================================================
bool IsSpineAnimationPlaying(SpineEntity *entity, int track)
{
	if (!entity || !entity->active || !entity->state)
		return false;
	if (track < 0 || track >= entity->state->tracksCount)
		return false;

	spTrackEntry *entry = entity->state->tracks[track];
	return (entry != NULL);
}

//=================================================================
// Set Animation Speed
//=================================================================
void SetSpineAnimationSpeed(SpineEntity *entity, float speed)
{
	if (!entity || !entity->active || !entity->state)
		return;
	entity->state->timeScale = speed;
}

//=================================================================
// Set Position
//=================================================================
void SetSpinePosition(SpineEntity *entity, float x, float y)
{
	if (!entity || !entity->active)
		return;
	entity->x = x;
	entity->y = y;
}

//=================================================================
// Get Position
//=================================================================
void GetSpinePosition(SpineEntity *entity, float *x, float *y)
{
	if (!entity || !entity->active)
		return;
	if (x)
		*x = entity->x;
	if (y)
		*y = entity->y;
}

//=================================================================
// Set Scale
//=================================================================
void SetSpineScale(SpineEntity *entity, float scale)
{
	if (!entity || !entity->active)
		return;
	entity->scale = scale;
}

//=================================================================
// Get Scale
//=================================================================
float GetSpineScale(SpineEntity *entity)
{
	if (!entity || !entity->active)
		return 0.0f;
	return entity->scale;
}

//=================================================================
// Set Flip
//=================================================================
void SetSpineFlip(SpineEntity *entity, bool flip_x, bool flip_y)
{
	if (!entity || !entity->active || !entity->skeleton)
		return;
	entity->flip_x = flip_x;
	entity->flip_y = flip_y;

	// Normally artists draw characters facing right in spine
	// If left just change
	// entity->skeleton->scaleX = flip_x ? -1.0f : 1.0f;
	// OR
	// SetSpineFlip(game_data.player, false, false);

	entity->skeleton->scaleX = flip_x ? 1.0f : -1.0f;
	entity->skeleton->scaleY = flip_y ? -1.0f : 1.0f;
}

//=================================================================
// Get Flip
//=================================================================
void GetSpineFlip(SpineEntity *entity, bool *flip_x, bool *flip_y)
{
	if (!entity || !entity->active)
		return;
	if (flip_x)
		*flip_x = entity->flip_x;
	if (flip_y)
		*flip_y = entity->flip_y;
}

//=================================================================
// Set Visibility
//=================================================================
void SetSpineVisible(SpineEntity *entity, bool visible)
{
	if (!entity || !entity->active)
		return;
	entity->visible = visible;
}

//=================================================================
// Get Visibility
//=================================================================
bool IsSpineVisible(SpineEntity *entity)
{
	if (!entity || !entity->active)
		return false;
	return entity->visible;
}

//=================================================================
// Get Bounding Box (Axis-Aligned)
//=================================================================
Rectangle GetSpineBounds(SpineEntity *entity)
{
	Rectangle bounds = {0, 0, 0, 0};
	if (!entity || !entity->active || !entity->skeleton)
		return bounds;

	// Compute skeleton bounds
	float min_x = INFINITY, min_y = INFINITY;
	float max_x = -INFINITY, max_y = -INFINITY;

	for (int i = 0; i < entity->skeleton->slotsCount; i++)
	{
		spSlot *slot = entity->skeleton->slots[i];
		if (!slot || !slot->attachment)
			continue;

		if (slot->attachment->type == SP_ATTACHMENT_REGION)
		{
			spRegionAttachment *region = (spRegionAttachment *)slot->attachment;
			float world[8];
			spRegionAttachment_computeWorldVertices(region, slot, world, 0, 2);

			for (int j = 0; j < 8; j += 2)
			{
				float x = world[j];
				float y = world[j + 1];
				if (x < min_x)
					min_x = x;
				if (x > max_x)
					max_x = x;
				if (y < min_y)
					min_y = y;
				if (y > max_y)
					max_y = y;
			}
		}
	}

	if (min_x != INFINITY)
	{
		bounds.x = entity->x + min_x * entity->scale;
		bounds.y = entity->y - max_y * entity->scale; // Flip Y
		bounds.width = (max_x - min_x) * entity->scale;
		bounds.height = (max_y - min_y) * entity->scale;
	}

	return bounds;
}

//=================================================================
// Get Skeleton Bounds (from Spine data)
//=================================================================
Rectangle GetSpineSkeletonBounds(SpineEntity *entity)
{
	// For now, same as GetSpineBounds
	// Could be enhanced with spSkeleton_getBounds() if needed
	return GetSpineBounds(entity);
}

//=================================================================
// Get Width
//=================================================================
float GetSpineWidth(SpineEntity *entity)
{
	if (!entity || !entity->active)
		return 0.0f;
	
	Rectangle bounds = GetSpineBounds(entity);
	return bounds.width;
}

//=================================================================
// Get Height
//=================================================================
float GetSpineHeight(SpineEntity *entity)
{
	if (!entity || !entity->active)
		return 0.0f;
	
	Rectangle bounds = GetSpineBounds(entity);
	return bounds.height;
}

//=================================================================
// Check Collision Between Entities
//=================================================================
bool CheckSpineCollision(SpineEntity *entity_lhs, SpineEntity *entity_rhs)
{
	if (!entity_lhs || !entity_rhs || !entity_lhs->active || !entity_rhs->active)
		return false;

	Rectangle bounds_lhs = GetSpineBounds(entity_lhs);
	Rectangle bounds_rhs = GetSpineBounds(entity_rhs);

	return CheckCollisionRecs(bounds_lhs, bounds_rhs);
}

//=================================================================
// Check Collision with Rectangle
//=================================================================
bool CheckSpineCollisionRect(SpineEntity *entity, Rectangle rectangle)
{
	if (!entity || !entity->active)
		return false;

	Rectangle bounds = GetSpineBounds(entity);
	return CheckCollisionRecs(bounds, rectangle);
}

//=================================================================
// Update Single Entity
//=================================================================
void UpdateSpineEntity(SpineEntity *entity, float delta_time)
{
	if (!entity || !entity->active || !entity->state || !entity->skeleton)
		return;

	spAnimationState_update(entity->state, delta_time);
	spAnimationState_apply(entity->state, entity->skeleton);
	spSkeleton_updateWorldTransform(entity->skeleton, SP_PHYSICS_NONE);
}

//=================================================================
// Update All Entities
//=================================================================
void UpdateAllSpineEntities(float delta_time)
{
	for (int i = 0; i < MAX_SPINE_ENTITIES; i++)
	{
		if (entities[i].active)
			UpdateSpineEntity(&entities[i], delta_time);
	}
}

//=================================================================
// Draw Single Entity
//=================================================================
void DrawSpineEntity(SpineEntity *entity)
{
	if (!entity || !entity->active || !entity->visible)
		return;
	if (!entity->skeleton || !entity->atlas)
		return;

	DrawSkeletonInternal(entity->skeleton, entity->atlas, entity->x, entity->y, entity->scale);

	// Debug rendering
	if (debug_mode)
	{
		Rectangle bounds = GetSpineBounds(entity);
		DrawRectangleLinesEx(bounds, 2.0f, RED);
	}
}

//=================================================================
// Draw All Entities
//=================================================================
void DrawAllSpineEntities()
{
	// Reset mesh pool slot counter
	ResetMeshPoolSlot();

// Reset debug quad counter
#if defined(DEBUG)
	ResetDebugQuads();
#endif

	BeginMode3D(spine_camera);
	rlDisableDepthTest();
	rlDisableBackfaceCulling();

	// Draw all visible entities
	for (int i = 0; i < MAX_SPINE_ENTITIES; i++)
	{
		if (entities[i].active)
			DrawSpineEntity(&entities[i]);
	}

	rlEnableBackfaceCulling();
	rlEnableDepthTest();
	EndMode3D();

// Draw debug quads AFTER 3D mode
#if defined(DEBUG)
	if (debug_mode)
	{
		for (int i = 0; i < debug_quad_count; i++)
		{
			DebugQuad *quad = &debug_quads[i];
			DrawLineV(quad->vertices[0], quad->vertices[1], quad->color);
			DrawLineV(quad->vertices[1], quad->vertices[2], quad->color);
			DrawLineV(quad->vertices[2], quad->vertices[3], quad->color);
			DrawLineV(quad->vertices[3], quad->vertices[0], quad->color);

			// Draw corner dots
			for (int j = 0; j < 4; j++)
				DrawCircleV(quad->vertices[j], 3.0f, quad->color);
		}
	}
#endif

	// Update stats
	manager_stats.pool_slots_used = model_cache_slot;
	manager_stats.pool_usage_percent = (model_cache_slot * 100.0f) / MAX_CACHED_MODELS;
}

//=================================================================
// Get Entity Count
//=================================================================
int GetSpineEntityCount()
{
	return manager_stats.entities_active;
}

//=================================================================
// Get Spine Manager Stats
//=================================================================
SpineManagerStats GetSpineManagerStats()
{
	return manager_stats;
}

//=================================================================
// Save Spine Settings
//=================================================================
bool SaveSpineSettings(const char *filepath)
{
	if (!manager_initialised)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Cannot save settings, not initialized");
		return false;
	}

	// TODO: Implement settings save (positions, scales, debug mode, etc.)
	// Format: key=value pairs
	// Example:
	// debug_mode=0
	// default_scale=1.0
	// pool_size=2000

	TraceLog(LOG_WARNING, "SPINE_MANAGER: SaveSpineSettings not yet implemented");
	return false;
}

//=================================================================
// Load Spine Settings
//=================================================================
bool LoadSpineSettings(const char *filepath)
{
	if (!manager_initialised)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Cannot load settings - not initialized");
		return false;
	}

	// TODO: Implement settings load
	// Parse key=value format

	TraceLog(LOG_WARNING, "SPINE_MANAGER: LoadSpineSettings not yet implemented");
	return false;
}

//=================================================================
// Enable/Disable Debug Mode
//=================================================================
void SetSpineDebugMode(bool enabled)
{
	debug_mode = enabled;
	TraceLog(LOG_INFO, "SPINE_MANAGER: Debug mode %s", enabled ? "ENABLED" : "DISABLED");
}

//=================================================================
// Get Debug Mode State
//=================================================================
bool IsSpineDebugMode()
{
	return debug_mode;
}

//=================================================================
// Cleanup Spine Manager
//=================================================================
void ExitSpineManager()
{
	if (!manager_initialised)
	{
		TraceLog(LOG_WARNING, "SPINE_MANAGER: Not initialised");
		return;
	}

	TraceLog(LOG_INFO, "SPINE_MANAGER: Shutting down");

	// Destroy all entities
	for (int i = 0; i < MAX_SPINE_ENTITIES; i++)
	{
		if (entities[i].active)
			DestroySpineEntity(&entities[i]);
	}

	// Cleanup mesh pool
	CleanupMeshPool();

	// Clear cached skeletons
	for (int i = 0; i < MAX_CACHED_SKELETONS; i++)
	{
		if (cached_skeletons[i].in_use)
		{
			if (cached_skeletons[i].skeleton_data)
				spSkeletonData_dispose(cached_skeletons[i].skeleton_data);
			if (cached_skeletons[i].atlas)
				spAtlas_dispose(cached_skeletons[i].atlas);
			cached_skeletons[i].in_use = false;
		}
	}

	manager_initialised = false;
	TraceLog(LOG_INFO, "SPINE_MANAGER: Shutdown complete");
}