#include <math.h>

#include "supermech.h"

static void Dormant_Entry(SuperMech *mech, float dt);
static void Dormant_Update(SuperMech *mech, float dt);
static void Dormant_Exit(SuperMech *mech, float dt);

static void Idle_Entry(SuperMech *mech, float dt);
static void Idle_Update(SuperMech *mech, float dt);
static void Idle_Exit(SuperMech *mech, float dt);

static void Hunt_Entry(SuperMech *mech, float dt);
static void Hunt_Update(SuperMech *mech, float dt);
static void Hunt_Exit(SuperMech *mech, float dt);

static void Search_Entry(SuperMech *mech, float dt);
static void Search_Update(SuperMech *mech, float dt);
static void Search_Exit(SuperMech *mech, float dt);

//------------------Math Helper Functions------------------//

static float Vec2Length(Vector2 v)
{
    return sqrtf(v.x * v.x + v.y * v.y);
}

static Vector2 Vec2Add(Vector2 a, Vector2 b) 
{
    return (Vector2){ a.x + b.x, a.y + b.y };
}

static Vector2 Vec2Sub(Vector2 a, Vector2 b) 
{
    return (Vector2){ a.x - b.x, a.y - b.y };
}

static Vector2 Vec2Scale(Vector2 v, float s) 
{
    return (Vector2){ v.x * s, v.y * s };
}

static Vector2 Vec2Normalize(Vector2 v) 
{
    float len = Vec2Length(v);
    if (len > 0.0001f) {
        return Vec2Scale(v, 1.0f / len);
    }
    return (Vector2){0, 0};
}

static float Vec2Distance(Vector2 a, Vector2 b) 
{
    return Vec2Length(Vec2Sub(a, b));
}

//------------------Other Helper Functions------------------//

static bool CanSeePlayer(const SuperMech *mech, Vector2 playerPos) 
{
    float dist = Vec2Distance(mech->position, playerPos);
    return dist <= mech->visionRange;
}

static void MoveTowards(SuperMech *mech, Vector2 target, float speed) 
{
    Vector2 dir = Vec2Sub(target, mech->position);

    if (Vec2Length(dir) > 1.0f) 
    {
        dir = Vec2Normalize(dir);
        mech->velocity = Vec2Scale(dir, speed);
        mech->position = Vec2Add(mech->position, mech->velocity);
    } 
    else 
    {
        mech->velocity = (Vector2){0, 0};
    }
}

//------------------FSM Functions------------------//

static bool CanEnterState(SuperMech *mech, SupermechState newState) 
{
    StateConfig *config = &mech->stateConfigs[mech->currentState];

    for (int i = 0; i < config->nextStatesCount; i++)
    {
        if (config->nextStates[i] == newState) return true;
    }

    return false;
}

static void ChangeState(SuperMech *mech, SupermechState newState, float dt) 
{
    if (!CanEnterState(mech, newState)) 
    {
        printf("Invalid state transition from %s to %s\n",
               mech->stateConfigs[mech->currentState].name,
               mech->stateConfigs[newState].name);
        return;
    }

    mech->currentFrame = 0;
    mech->animationTimer = 0.0f;

    StateConfig *current = &mech->stateConfigs[mech->currentState];
    StateConfig *next = &mech->stateConfigs[newState];

    if (current->Exit) current->Exit(mech, dt);

    mech->previousState = mech->currentState;
    mech->currentState = newState;
    mech->stateTimer = 0.0f;

    if (next->Entry) next->Entry(mech, dt);
}

static void UpdateState(SuperMech *mech, float dt) 
{
    StateConfig *current = &mech->stateConfigs[mech->currentState];
    if (current->Update) current->Update(mech, dt);
}

//------------------Public Functions------------------//

void SuperMech_Init(SuperMech *mech, Vector2 startPos) 
{
    mech->position = startPos;
    mech->velocity = (Vector2){0,0};

    mech->speedIdle = 1.2f;
    mech->speedHunt = 3.5f;
    mech->visionRange = 300.0f;

    mech->stateTimer = 0.0f;
    mech->lastKnownPlayerPos = startPos;
    mech->playerVisible = false;

    mech->currentState = MECH_DORMANT;
    mech->previousState = MECH_DORMANT;

    mech->texture = LoadTexture("./assets/supermech/supermech_64x98.png");
    mech->frameWidth  = 64;
    mech->frameHeight = 98;
    mech->scale = 1.0f;

    mech->frameCount = 8;     //frames per row
    mech->frameTime  = 0.12f; //seconds per frame
    mech->currentFrame = 0;
    mech->animationTimer = 0.0f;
    mech->facingRight = true;

    //-------Configure States-------//

    //Dormant
    static int dormantNext[] = {MECH_IDLE, MECH_HUNT};
    mech->stateConfigs[MECH_DORMANT] = (StateConfig){
        "Dormant",
        NULL,
        Dormant_Entry,
        Dormant_Update,
        Dormant_Exit,
        dormantNext, sizeof(dormantNext)/sizeof(int)
    };

    //Idle
    static int idleNext[] = {MECH_DORMANT, MECH_SEARCH};
    mech->stateConfigs[MECH_IDLE] = (StateConfig){
        "Idle",
        NULL,
        Idle_Entry,
        Idle_Update,
        Idle_Exit,
        idleNext, sizeof(idleNext)/sizeof(int)
    };

    //Hunt
    static int huntNext[] = {MECH_SEARCH, MECH_IDLE};
    mech->stateConfigs[MECH_HUNT] = (StateConfig){
        "Hunt",
        NULL,
        Hunt_Entry,
        Hunt_Update,
        Hunt_Exit,
        huntNext, sizeof(huntNext)/sizeof(int)
    };

    //Search
    static int searchNext[] = {MECH_IDLE, MECH_HUNT};
    mech->stateConfigs[MECH_SEARCH] = (StateConfig){
        "Search",
        NULL,
        Search_Entry,
        Search_Update,
        Search_Exit,
        searchNext, sizeof(searchNext)/sizeof(int)
    };
}

void SuperMech_Update(SuperMech *mech, Vector2 playerPos, bool cameraTriggered, float dt) 
{
    mech->playerVisible = CanSeePlayer(mech, playerPos);

    if (cameraTriggered && mech->currentState == MECH_DORMANT)
    {
        ChangeState(mech, MECH_HUNT, dt);
    }

    mech->lastKnownPlayerPos = playerPos;

    UpdateState(mech, dt);
}

void SuperMech_Draw(SuperMech *mech) 
{
    mech->animationTimer += GetFrameTime();
    if (mech->animationTimer >= mech->frameTime)
    {
        mech->animationTimer -= mech->frameTime;
        mech->currentFrame++;

        if (mech->currentFrame >= mech->frameCount)
        {
            mech->currentFrame = 0;
        }
    }

    float frameWidth = (float)mech->frameWidth;
    float sourceX = (float)(mech->currentFrame * mech->frameWidth);

    if (!mech->facingRight)
    {
        frameWidth *= -1;
        sourceX += mech->frameWidth;
    }

    Rectangle source = { sourceX, 0.0f, frameWidth, (float)mech->frameHeight };

    Vector2 origin = {
        (mech->frameWidth * mech->scale) / 2.0f,
        (mech->frameHeight * mech->scale) / 2.0f
    };

    Rectangle dest = {
        mech->position.x + origin.x,
        mech->position.y + origin.y,
        mech->frameWidth * mech->scale,
        mech->frameHeight * mech->scale
    };

    DrawTexturePro( mech->texture, source, dest, origin, 0.0f, WHITE);
}

const char *SuperMech_GetStateName(SupermechState state) 
{
    switch(state) {
        case MECH_DORMANT: return "Dormant";
        case MECH_IDLE: return "Idle";
        case MECH_HUNT: return "Hunt";
        case MECH_SEARCH: return "Search";
        default: return "Unknown";
    }
}

//------------------Dormant------------------//

static void Dormant_Entry(SuperMech *mech, float dt)
{
    mech->velocity = (Vector2){0, 0};
}

static void Dormant_Update(SuperMech *mech, float dt)
{
    // do nothing
}

static void Dormant_Exit(SuperMech *mech, float dt)
{
}

//------------------Idle------------------//

static void Idle_Entry(SuperMech *mech, float dt)
{
}

static void Idle_Update(SuperMech *mech, float dt)
{
}

static void Idle_Exit(SuperMech *mech, float dt)
{
}

//------------------Hunt------------------//

static void Hunt_Entry(SuperMech *mech, float dt)
{
}

static void Hunt_Update(SuperMech *mech, float dt)
{
    MoveTowards(mech, mech->lastKnownPlayerPos, mech->speedHunt);
}

static void Hunt_Exit(SuperMech *mech, float dt)
{
}

//------------------Search------------------//

static void Search_Entry(SuperMech *mech, float dt)
{
}

static void Search_Update(SuperMech *mech, float dt)
{
}

static void Search_Exit(SuperMech *mech, float dt)
{
}