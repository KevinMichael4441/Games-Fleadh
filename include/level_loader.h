#ifndef LEVEL_LOADER_H
#define LEVEL_LOADER_H

#include "cJSON.h"
#include "raylib.h"

#ifdef __cplusplus
extern "C" {
#endif

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


#ifdef __cplusplus
}
#endif

#endif