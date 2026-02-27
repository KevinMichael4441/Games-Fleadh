#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include "cJSON.h"
#include "raylib.h"
#include "constants.h"
#include <math.h>
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ChunkCache
{
    RenderTexture2D tex[9];
} ChunkCache;

typedef struct LevelObject
{
    const char* type;
    float x;
    float y;
    cJSON* properties;
} LevelObject;

typedef struct
{
    int levelWidth;             // number of tiles in the width of the level
    int levelHeight;            // number of tiles in the height of the level
    int tileWidth;
    int tileHeight;

    int* boundaryTiles;         // pointer to an array that holds collision tiles

    Texture2D tilesetTexture;   // image for holding tile art

    int tilesetColumns;         // number of columns in the tileset
    int firstGid;               // first tile ID used by Tiled
    int tileCount;

    cJSON* mapRoot;
    cJSON* levelLayer;
    cJSON* foregroundLayer;
    cJSON* boundaryLayer;
    cJSON* backgroundLayer;
    cJSON* objectLayer;

    int* levelGids;
    int* foregroundGids;
    int* boundaryGids;
    int* backgroundGids;

    int chunkWidthPx;           // chunk pixel size (screen size)
    int chunkHeightPx;
    int chunksX;                // num of chunks
    int chunksY;
    int centreChunkX;           // the centre of the 9 chunks
    int centreChunkY;

    ChunkCache levelCache;
    ChunkCache backgroundCache;
    int slotTex[3][3];              // storage for the individual chunks 0-8
    unsigned char slotValid[3][3];  // used for edge cases where the player (centre chunk) is at the edge of the map so adjacent chunks could be out of bounds
    unsigned char chunkCacheReady; 

    LevelObject* objects;
    int objectCount;
} LevelData;


// loads a json file from disk and converts it into a usable... object?
cJSON* LoadJsonFile(const char* filename);

// finds the data from a specific layer
cJSON* FindTileLayerDataArray(cJSON* levelJson, const char* layerName);

// draws a tile layer
void DrawTileLayer(const LevelData* level, cJSON* layerDataArray);

// combine a folder path and filename into one string
void JoinPath(char* outputPath, int outputBufferSize, const char* baseDir, const char* fileName);

// get tileset information from json
bool ReadTilesetInfoFromMapJson(LevelData* level, cJSON* mapRoot, const char* mapBaseDir);

// load the level
bool Level_Load(LevelData* level, const char* mapPath, const char* mapBaseDir, const char* tilesetPngPath);

// unload the level
void Level_Unload(LevelData* level);

// Returns true if the world position (x,y) is inside a solid boundary tile.
bool Level_IsBoundaryPos(const LevelData* level, float posX, float posY);

// sets up 9 rdender textures
bool chunkCacheInit(LevelData* level, int chunkW_px, int chunkH_px);

// releases resources
void chunkCacheUnload(LevelData* level);

// updates chunks based on player world position
void chunkCacheUpdate(LevelData* level, Vector2 playerWorldPos);

// draws the chunks
void chunkCacheDraw(const LevelData* level);

void chunkCacheDrawBackground(const LevelData* level);


bool LevelLoadObjects(LevelData* level, const char* objectLayerName);


#ifdef __cplusplus
}
#endif

#endif