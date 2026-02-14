#include "level_loader.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


cJSON* LoadJsonFile(const char* filename)
{
	char* fileText = LoadFileText(filename); // read the file to string

	if(fileText == NULL)
	{
		TraceLog(LOG_ERROR, "LoadFileText failed (file not found): %s", filename);
		return NULL;
	}

	cJSON* root = cJSON_Parse(fileText); // make it usable? convert string to useful data

	UnloadFileText(fileText);

	if (root == NULL)
	{
		TraceLog(LOG_ERROR, "cJSON_Parse failed (invalid JSON?) in file: %s", filename);
		return NULL;
	}

	return root;
}

cJSON* FindTileLayerDataArray(cJSON* levelJson, const char* layerName)
{
	cJSON* layers = cJSON_GetObjectItem(levelJson, "layers");   // get the list of layers
	if (!layers || !cJSON_IsArray(layers))
    {
        TraceLog(LOG_ERROR, "Map JSON has no 'layers' array");
        return NULL;
    }

	int layerCount = cJSON_GetArraySize(layers);        // how many layers are there

	for (int i = 0; i < layerCount; i++)                // look for the layer we want
	{
		cJSON* layer = cJSON_GetArrayItem(layers, i);   // the layer object
		if (!layer)
		{
			continue;
		}

		cJSON* nameItem = cJSON_GetObjectItem(layer, "name");   // check that layers name for the one we want
		if (!nameItem || !cJSON_IsString(nameItem))
		{
			continue;
		}

		if (strcmp(nameItem->valuestring, layerName) != 0)     // skip if its not
		{
			continue;
		}

		cJSON* typeItem = cJSON_GetObjectItem(layer, "type");   // confirm that it is a layer
		if (!typeItem || !cJSON_IsString(typeItem) || strcmp(typeItem->valuestring, "tilelayer") != 0)
		{
			TraceLog(LOG_ERROR, "Layer '%s' exists but is not a tilelayer", layerName);
            return NULL;
		}

		cJSON* dataArray = cJSON_GetObjectItem(layer, "data");  // get the data from the layer
		if (!dataArray || !cJSON_IsArray(dataArray))
		{
			TraceLog(LOG_ERROR, "Layer '%s' has no 'data' array", layerName);
            return NULL;
		}

		return dataArray;
	}

	TraceLog(LOG_ERROR, "Could not find layer named '%s'", layerName);
    return NULL;
}

bool ReadTilesetInfoFromMapJson(LevelData* level, cJSON* mapRoot, const char* mapBaseDir)
{
    if (!level || !mapRoot || !mapBaseDir)
        return false;

    cJSON* tilesets = cJSON_GetObjectItem(mapRoot, "tilesets");     // get tilesets from file
    if (!tilesets || !cJSON_IsArray(tilesets))
    {
        TraceLog(LOG_ERROR, "Map JSON has no 'tilesets' array");
        return false;
    }

    int count = cJSON_GetArraySize(tilesets);       // make sure there is one only tileset
    if (count != 1)
    {
        TraceLog(LOG_ERROR, "Expected exactly 1 tileset but map has %d", count);
        return false;
    }

    cJSON* tsRef = cJSON_GetArrayItem(tilesets, 0);     // get the only tileset
    if (!tsRef)
    {
        return false;
    }

    cJSON* firstgid = cJSON_GetObjectItem(tsRef, "firstgid");   // where tileset starts
    cJSON* source = cJSON_GetObjectItem(tsRef, "source");   // get filename of tileset from file

    if (!cJSON_IsNumber(firstgid) || !cJSON_IsString(source))
    {
        TraceLog(LOG_ERROR, "Tileset entry missing 'firstgid' or 'source'");
        return false;
    }

    char tilesetPath[512];
    JoinPath(tilesetPath, sizeof(tilesetPath), mapBaseDir, source->valuestring);    // Build full path to tileset JSON file


    cJSON* tsJson = LoadJsonFile(tilesetPath);     // open our tileset
    if (!tsJson)
    {
        TraceLog(LOG_ERROR, "Failed to load tileset JSON: %s", tilesetPath);
        return false;
    }

    cJSON* columns = cJSON_GetObjectItem(tsJson, "columns");        // number of cols in the tileset image
    cJSON* tilecount = cJSON_GetObjectItem(tsJson, "tilecount");    // total number of tiles in the tileset image

    if (!cJSON_IsNumber(columns) || !cJSON_IsNumber(tilecount))
    {
        TraceLog(LOG_ERROR, "Tileset JSON missing 'columns' or 'tilecount'");
        cJSON_Delete(tsJson);
        return false;
    }

    level->firstGid = firstgid->valueint;
    level->tilesetColumns = columns->valueint;
    level->tileCount = tilecount->valueint;

    cJSON_Delete(tsJson);

    return true;
}

void DrawTileLayer(const LevelData* level, cJSON* layerDataArray)
{
    if (!level || !layerDataArray) return;

    for (int y = 0; y < level->levelHeight; y++)
    {
        for (int x = 0; x < level->levelWidth; x++)
        {
            int index = y * level->levelWidth + x;      // convert 2d tile coords to flat array (not sure I fully understand this)

            cJSON* tileItem = cJSON_GetArrayItem(layerDataArray, index);    // get gid from array
            if (!tileItem || !cJSON_IsNumber(tileItem))
            {
                continue;
            } 

            int gid = tileItem->valueint; 
            if (gid == 0)
            {
                continue;
            }

            gid = gid & 0x1FFFFFFF;     // remove Tiled flip flags

            int localId = gid - level->firstGid;
            if (localId < 0)
            {
                continue;
            }

            int srcCol = localId % level->tilesetColumns;
            int srcRow = localId / level->tilesetColumns;

            Rectangle src = {(float)(srcCol * level->tileWidth), (float)(srcRow * level->tileHeight), (float)level->tileWidth, (float)level->tileHeight};

            Vector2 dst = {(float)(x * level->tileWidth),(float)(y * level->tileHeight)};

            DrawTextureRec(level->tilesetTexture, src, dst, WHITE);
        }
    }
}

void JoinPath(char* outBuffer, int outSize, const char* baseDir, const char* file)
{
    snprintf(outBuffer, outSize, "%s%s", baseDir, file);
}

bool Level_Load(LevelData* level, const char* mapPath, const char* mapBaseDir, const char* tilesetPngPath)
{
    if (!level) return false;

    *level = (LevelData){0};

    level->tileWidth = 32;
    level->tileHeight = 32;

    level->mapRoot = LoadJsonFile(mapPath);
    if (!level->mapRoot) return false;

    cJSON* w = cJSON_GetObjectItem(level->mapRoot, "width");
    cJSON* h = cJSON_GetObjectItem(level->mapRoot, "height");
    if (!w || !h || !cJSON_IsNumber(w) || !cJSON_IsNumber(h))
    {
        return false;
    }

    level->levelWidth  = w->valueint;
    level->levelHeight = h->valueint;

    if (!ReadTilesetInfoFromMapJson(level, level->mapRoot, mapBaseDir))
    {
        return false;
    }

    level->tilesetTexture = LoadTexture(tilesetPngPath);
    if (level->tilesetTexture.id == 0)
    {
        return false;
    }

    level->levelLayer = FindTileLayerDataArray(level->mapRoot, "Level");
    level->foregroundLayer = FindTileLayerDataArray(level->mapRoot, "Foreground");
    level->boundaryLayer = FindTileLayerDataArray(level->mapRoot, "Boundary");

    if (!level->levelLayer)
        return false;

    return true;
}

void Level_Unload(LevelData* level)
{
    if (!level) return;

    if (level->tilesetTexture.id != 0)
    {
        UnloadTexture(level->tilesetTexture);
    }

    if (level->boundaryTiles)
    {
        free(level->boundaryTiles);
    }

    if (level->mapRoot)
    {
        cJSON_Delete(level->mapRoot);
    }

    *level = (LevelData){0};
}