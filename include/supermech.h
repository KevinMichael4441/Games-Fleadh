#ifndef SUPERMECH_H
#define SUPERMECH_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "raylib.h"
#include "cute_c2.h"

#include <math.h>

#include "level_loader.h"

#define MAX_BOUNDARY_RECTS 16

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum {
    MECH_DORMANT,
    MECH_IDLE,
    MECH_HUNT,
    MECH_SEARCH,
    MECH_STATE_COUNT
} SupermechState; //for FSM

struct SuperMech;
typedef void (*StateFunction)(struct SuperMech *, float);
typedef void (*EventFunction)(struct SuperMech *, float);

typedef struct {
    const char *name;
    EventFunction HandleEvent;  //optional, can be NULL

    StateFunction Entry;
    StateFunction Update;
    StateFunction Exit;

    int *nextStates;
    int nextStatesCount;
} StateConfig;

typedef struct SuperMech {
    LevelData currentLevel;

    Vector2 position;
    Vector2 velocity;

    float speedIdle;
    float speedHunt;
    float visionRange;

    float gravity;
    bool isGrounded;
    float jumpForce;

    float stateTimer;
    Vector2 lastKnownPlayerPos;
    bool playerVisible;

    //---Sprite Properties---//
    Texture2D* currentTexture;
    Texture2D textureDormant;
    Texture2D textureIdle;
    Texture2D textureHunt;
    Texture2D textureSearch;
    int frameWidth;
    int frameHeight;
    float scale;

    //---Animation---//
    int frameCount;
    float frameTime;
    int currentFrame;
    float animationTimer;
    bool facingRight;

    //---FSM---//
    SupermechState currentState;
    SupermechState previousState;
    StateConfig stateConfigs[MECH_STATE_COUNT];
} SuperMech; //actual SM

void SuperMech_Init(SuperMech *mech, Vector2 startPos, LevelData* level);

void SuperMech_Uppdate(SuperMech *mech, Vector2 playerPos, bool cameraTriggered, float dt);
void SuperMech_Draw(SuperMech *mech);

bool SuperMech_CheckCollision_Player(const SuperMech *mech, Vector2 center, float radius);
void SuperMech_Reset(SuperMech *mech, Vector2 startPos);

const char *SuperMech_GetStateName(SupermechState state);

#ifdef __cplusplus
}
#endif

#endif //SUPERMECH_H