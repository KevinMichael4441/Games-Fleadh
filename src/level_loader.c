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

static bool ConvertTileLayerToArray(LevelData* level, cJSON* layerDataArray, int** newArray)
{
    if (!level || !layerDataArray || !newArray)
    {
        return false;
    }

    // num tiles in map
    int count = level->levelWidth * level->levelHeight;

    // get memory for that many ints 
    int* arr = (int*)malloc(sizeof(int) * count);
    if (!arr)
    {
        TraceLog(LOG_ERROR, "Out of memory allocating tile array (%d ints)", count);
        return false;
    }

    for (int i = 0; i < count; i++)
    {
        cJSON* id = cJSON_GetArrayItem(layerDataArray, i); // getthe tile value from the json array
        int gid;    // store the gid for that tile

        if (id && cJSON_IsNumber(id))   // if it exists and is a number
        {
            gid = id->valueint;         // get the actual tile number
        }
        else
        {
            gid = 0;
        }
            gid &= 0x1FFFFFFF;          // remove the flip flag
            arr[i] = gid;               // store in our array
        }

    *newArray = arr;
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

static void drawTileLayerChunk(const LevelData* level, cJSON* layerDataArray, int originPxX, int originPxY);

void JoinPath(char* outBuffer, int outSize, const char* baseDir, const char* file)
{
    snprintf(outBuffer, outSize, "%s%s", baseDir, file);
}

bool Level_Load(LevelData* level, const char* mapPath, const char* mapBaseDir, const char* tilesetPngPath)
{
    if (!level)
    {
        return false;
    }

    *level = (LevelData){0};

    level->tileWidth = 32;
    level->tileHeight = 32;

    level->mapRoot = LoadJsonFile(mapPath);
    if (!level->mapRoot)
    {
        return false;
    }

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
    {
        return false;
    }

        if (!ConvertTileLayerToArray(level, level->levelLayer, &level->levelGids))
    {
        Level_Unload(level);
        return false;
    }

    if (level->foregroundLayer)
    {
        if (!ConvertTileLayerToArray(level, level->foregroundLayer, &level->foregroundGids))
        {
            Level_Unload(level);
            return false;
        }
    }

    if (level->boundaryLayer)
    {
        if (!ConvertTileLayerToArray(level, level->boundaryLayer, &level->boundaryGids))
        {
            Level_Unload(level);
            return false;
        }
    }

    return true;
}

void Level_Unload(LevelData* level)
{
    if (!level) return;

    chunkCacheUnload(level);

    if (level->tilesetTexture.id != 0)
    {
        UnloadTexture(level->tilesetTexture);
        level->tilesetTexture.id = 0;
    }

    if (level->boundaryTiles)
    {
        free(level->boundaryTiles);
        level->boundaryTiles = NULL;
    }

    if (level->levelGids)
    {
        free(level->levelGids);
        level->levelGids = NULL;
    }

    if (level->foregroundGids)
    {
        free(level->foregroundGids);
        level->foregroundGids = NULL;
    }

    if (level->boundaryGids)
    {
        free(level->boundaryGids);
        level->boundaryGids = NULL;
    }

    if (level->mapRoot)
    {
        cJSON_Delete(level->mapRoot);
        level->mapRoot = NULL;
    }

    *level = (LevelData){0};
}

bool Level_IsBoundaryPos(const LevelData* level, float posX, float posY)
{
    if (!level || !level->boundaryLayer)
        return false;

    int tx = (int)(posX / level->tileWidth);
    int ty = (int)(posY / level->tileHeight);

    int index = ty * level->levelWidth + tx;

    cJSON* tileItem = cJSON_GetArrayItem(level->boundaryLayer, index);
    if (!tileItem || !cJSON_IsNumber(tileItem))
    {
        return false;
    }

    int gid = tileItem->valueint;

    if (gid == 0)
    {
        return false;
    }

    gid = gid & 0x1FFFFFFF; // remove Tiled flip flags

    return true;
}

static void SlotMappingReset(LevelData* level)
{
    int i = 0;
    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            level->slotTex[r][c] = i++;
            level->slotValid[r][c] = 0;
        }
    }
}

static bool chunkInBounds(const LevelData* level, int chunkX, int chunkY)
{
    return (chunkX >= 0 && chunkX < level->chunksX && chunkY >= 0 && chunkY < level->chunksY);
}

static void bakeChunkIntoTexture(LevelData* level, cJSON* layerDataArray, ChunkCache* cache, int worldChunkX, int worldChunkY, int texIndex)
{
    BeginTextureMode(cache->tex[texIndex]);
    ClearBackground(BLANK);

    // ignore if out of bounds
    if (chunkInBounds(level, worldChunkX, worldChunkY))
    {
        int originPxX = worldChunkX * level->chunkWidthPx;
        int originPxY = worldChunkY * level->chunkHeightPx;

        if (layerDataArray)
        {
            drawTileLayerChunk(level, layerDataArray, originPxX, originPxY);
        }
    }

    EndTextureMode();
}

static void bakeChunkLevelLayer(LevelData* level, int wcx, int wcy, int texIndex)
{
    bakeChunkIntoTexture(level, level->levelLayer, &level->levelCache, wcx, wcy, texIndex);
}

static void drawTileLayerChunk(const LevelData* level, cJSON* layerDataArray, int originPxX, int originPxY)
{
    if (!level || !layerDataArray)
    {
        return;
    }

    // convert chunk world position to tile coords
    int startTileX = originPxX / level->tileWidth;
    int startTileY = originPxY / level->tileHeight;

    // calculate chunks end position in world pixels
    int endPxX = originPxX + level->chunkWidthPx;
    int endPxY = originPxY + level->chunkHeightPx;

    // convert end position to tile coords
    int endTileX = (endPxX + level->tileWidth - 1) / level->tileWidth;
    int endTileY = (endPxY + level->tileHeight - 1) / level->tileHeight;

    if (endTileX <= 0 || endTileY <= 0 || startTileX >= level->levelWidth || startTileY >= level->levelHeight)
    {
        return;
    }

    if (startTileX < 0)
    {
        startTileX = 0;
    }
    if (startTileY < 0)
    {
        startTileY = 0;
    }

    if (endTileX > level->levelWidth)
    {
        endTileX = level->levelWidth;
    }
    if (endTileY > level->levelHeight)
    {
        endTileY = level->levelHeight;
    }

    for (int y = startTileY; y < endTileY; y++)
    {
        for (int x = startTileX; x < endTileX; x++)
        {
            int index = y * level->levelWidth + x;

            int gid = level->levelGids[index];
            if (gid == 0)
            {
                continue;
            }

            gid = gid & 0x1FFFFFFF; // remove flip flags

            int localId = gid - level->firstGid;
            if (localId < 0)
            {
                continue;
            }

            int srcCol = localId % level->tilesetColumns;
            int srcRow = localId / level->tilesetColumns;

            Rectangle src = {(float)(srcCol * level->tileWidth), (float)(srcRow * level->tileHeight), (float)level->tileWidth, (float)level->tileHeight};

            Vector2 dst = {(float)(x * level->tileWidth - originPxX), (float)(y * level->tileHeight - originPxY)};

            DrawTextureRec(level->tilesetTexture, src, dst, WHITE);
        }
    }
}

static void rebuildAll3x3(LevelData* level, int newCentreChunkX, int newCentreChunkY)
{
    level->centreChunkX = newCentreChunkX;
    level->centreChunkY = newCentreChunkY;

    for (int r = 0; r < 3; r++)
    {
        for (int c = 0; c < 3; c++)
        {
            int wcx = newCentreChunkX + (c - 1);
            int wcy = newCentreChunkY + (r - 1);

            level->slotValid[r][c] = chunkInBounds(level, wcx, wcy);

            int texIndex = level->slotTex[r][c];
            bakeChunkLevelLayer(level, wcx, wcy, texIndex);
        }
    }
}

static void shiftRight(LevelData* level)
{
    for (int r = 0; r < 3; r++)
    {
        int freedTex = level->slotTex[r][0];
        level->slotTex[r][0] = level->slotTex[r][1];
        level->slotTex[r][1] = level->slotTex[r][2];
        level->slotTex[r][2] = freedTex;

        int freedValid = level->slotValid[r][0];
        level->slotValid[r][0] = level->slotValid[r][1];
        level->slotValid[r][1] = level->slotValid[r][2];
        level->slotValid[r][2] = freedValid;
    }

    level->centreChunkX += 1;

    // bake new right column
    int c = 2;
    for (int r = 0; r < 3; r++)
    {
        int wcx = level->centreChunkX + (c - 1);
        int wcy = level->centreChunkY + (r - 1);

        level->slotValid[r][c] = chunkInBounds(level, wcx, wcy);

        int texIndex = level->slotTex[r][c];
        bakeChunkLevelLayer(level, wcx, wcy, texIndex);
    }
}

static void shiftLeft(LevelData* level)
{
    for (int r = 0; r < 3; r++)
    {
        int freedTex = level->slotTex[r][2];
        level->slotTex[r][2] = level->slotTex[r][1];
        level->slotTex[r][1] = level->slotTex[r][0];
        level->slotTex[r][0] = freedTex;

        int freedValid = level->slotValid[r][2];
        level->slotValid[r][2] = level->slotValid[r][1];
        level->slotValid[r][1] = level->slotValid[r][0];
        level->slotValid[r][0] = freedValid;
    }

    level->centreChunkX -= 1;

    int c = 0;
    for (int r = 0; r < 3; r++)
    {
        int wcx = level->centreChunkX + (c - 1);
        int wcy = level->centreChunkY + (r - 1);

        level->slotValid[r][c] = chunkInBounds(level, wcx, wcy);

        int texIndex = level->slotTex[r][c];
        bakeChunkLevelLayer(level, wcx, wcy, texIndex);
    }
}

static void shiftDown(LevelData* level)
{
    for (int c = 0; c < 3; c++)
    {
        int freedTex = level->slotTex[0][c];
        level->slotTex[0][c] = level->slotTex[1][c];
        level->slotTex[1][c] = level->slotTex[2][c];
        level->slotTex[2][c] = freedTex;

        int freedValid = level->slotValid[0][c];
        level->slotValid[0][c] = level->slotValid[1][c];
        level->slotValid[1][c] = level->slotValid[2][c];
        level->slotValid[2][c] = freedValid;
    }

    level->centreChunkY += 1;

    int r = 2;
    for (int c = 0; c < 3; c++)
    {
        int wcx = level->centreChunkX + (c - 1);
        int wcy = level->centreChunkY + (r - 1);

        level->slotValid[r][c] = chunkInBounds(level, wcx, wcy);

        int texIndex = level->slotTex[r][c];
        bakeChunkLevelLayer(level, wcx, wcy, texIndex);
    }
}

static void shiftUp(LevelData* level)
{
    for (int c = 0; c < 3; c++)
    {
        int freedTex = level->slotTex[2][c];
        level->slotTex[2][c] = level->slotTex[1][c];
        level->slotTex[1][c] = level->slotTex[0][c];
        level->slotTex[0][c] = freedTex;

        int freedValid = level->slotValid[2][c];
        level->slotValid[2][c] = level->slotValid[1][c];
        level->slotValid[1][c] = level->slotValid[0][c];
        level->slotValid[0][c] = freedValid;
    }

    level->centreChunkY -= 1;

    int r = 0;
    for (int c = 0; c < 3; c++)
    {
        int wcx = level->centreChunkX + (c - 1);
        int wcy = level->centreChunkY + (r - 1);

        level->slotValid[r][c] = chunkInBounds(level, wcx, wcy);

        int texIndex = level->slotTex[r][c];
        bakeChunkLevelLayer(level, wcx, wcy, texIndex);
    }
}

bool chunkCacheInit(LevelData* level, int chunkWidthPx, int chunkHeightPx)
{
    if (!level)
    {
        return false;
    }

    level->chunkWidthPx = chunkWidthPx;
    level->chunkHeightPx = chunkHeightPx;

    // map size in pixels
    int mapWidthPx = level->levelWidth  * level->tileWidth;
    int mapHeightPx = level->levelHeight * level->tileHeight;

    // number of chunks in the the map
    level->chunksX = mapWidthPx / chunkWidthPx;
    level->chunksY = mapHeightPx / chunkHeightPx;

    // create 9 render textures
    for (int i = 0; i < 9; i++)
    {
        level->levelCache.tex[i] = LoadRenderTexture(chunkWidthPx, chunkHeightPx);
        BeginTextureMode(level->levelCache.tex[i]);
        ClearBackground(BLANK);
        EndTextureMode();
    }

    // set up the order starting order of the textures
    SlotMappingReset(level);

    // impossible value to force rebuild on first update
    level->centreChunkX = -1;
    level->centreChunkY = -1;

    level->chunkCacheReady = 1;
    return true;
}

void chunkCacheUnload(LevelData* level)
{
    if (!level || !level->chunkCacheReady)
    {
        return;
    }

    for (int i = 0; i < 9; i++)
    {
        UnloadRenderTexture(level->levelCache.tex[i]);
    }

    level->chunkCacheReady = 0;
}

void chunkCacheUpdate(LevelData* level, Vector2 playerWorldPos)
{
    if (!level || !level->chunkCacheReady)
    {
        return;
    }

    // calculate which chunk player is in from player position
    int playerChunkX = (int)floorf(playerWorldPos.x / (float)level->chunkWidthPx);
    int playerChunkY = (int)floorf(playerWorldPos.y / (float)level->chunkHeightPx);

    // first time or invalid rebuild all 9 around the player
    if (level->centreChunkX < 0 || level->centreChunkX >= level->chunksX || level->centreChunkY < 0 || level->centreChunkY >= level->chunksY)
    {
        rebuildAll3x3(level, playerChunkX, playerChunkY);
        return;
    }

    // check the players position relative to the chunks (have they crossed a boundary between chunks?)
    int dx = playerChunkX - level->centreChunkX;
    int dy = playerChunkY - level->centreChunkY;

    // update chunks if we cross a boundary between chunks
    if (dx == 1)
    {
        shiftRight(level);
    }
    else if (dx == -1)
    {
        shiftLeft(level);
    }

    if (dy == 1)
    {
        shiftDown(level);
    }
    else if (dy == -1)
    {
        shiftUp(level);
    }
}

void chunkCacheDraw(const LevelData* level)
{
    if (!level || !level->chunkCacheReady)
    {
        return;
    }

    // draw the chunks
    for (int row = 0; row < 3; row++)
    {
        for (int col = 0; col < 3; col++)
        {
            if (!level->slotValid[row][col])
            {
                continue;
            }

            // convert slot grid position to an offset from the centre slot
            int wcx = level->centreChunkX + (col - 1);
            int wcy = level->centreChunkY + (row - 1);

            // convert chunk coords to world pixel coords for drawing
            float worldX = (float)(wcx * level->chunkWidthPx);
            float worldY = (float)(wcy * level->chunkHeightPx);

            int texIndex = level->slotTex[row][col];
            RenderTexture2D rt = level->levelCache.tex[texIndex];

            Rectangle src = { 0, 0, (float)level->chunkWidthPx, -(float)level->chunkHeightPx };
            Vector2 dst = { worldX, worldY };

            DrawTextureRec(rt.texture, src, dst, WHITE);
        }
    }
}