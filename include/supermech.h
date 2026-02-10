#ifndef SUPERMECH_H
#define SUPERMECH_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <math.h>

#include "raylib.h"

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
    EventFunction HandleEvent;  // optional, can be NULL

    StateFunction Entry;
    StateFunction Update;
    StateFunction Exit;

    int *nextStates;
    int nextStatesCount;
} StateConfig;

typedef struct SuperMech {
    Vector2 position;
    Vector2 velocity;

    float speedIdle;
    float speedHunt;
    float visionRange;

    float stateTimer;
    Vector2 lastKnownPlayerPos;
    bool playerVisible;

    // FSM data
    SupermechState currentState;
    SupermechState previousState;
    StateConfig stateConfigs[MECH_STATE_COUNT];
} SuperMech; //actual SM

void SuperMech_Init(SuperMech *mech, Vector2 startPos);

void SuperMech_Update(SuperMech *mech, Vector2 playerPos, bool cameraTriggered, float dt);
void SuperMech_Draw(const SuperMech *mech);

const char *SuperMech_GetStateName(SupermechState state);

#endif //SUPERMECH_H