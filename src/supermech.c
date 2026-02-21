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

static void MoveTowards(SuperMech *mech, Vector2 target, float speed, float dt)
{
    Vector2 dir = Vec2Sub(target, mech->position);
    float dist = Vec2Length(dir);

    if (dist > 1.0f)
    {
        dir = Vec2Normalize(dir);

        Vector2 desiredVelocity = Vec2Scale(dir, speed);
        Vector2 steering = Vec2Sub(desiredVelocity, mech->velocity);

        mech->velocity = Vec2Add(mech->velocity, Vec2Scale(steering, 4.0f * dt));
        mech->position = Vec2Add(mech->position, Vec2Scale(mech->velocity, dt));

        mech->facingRight = (mech->velocity.x > 0);
    }
}

static float ClampFloat(float value, float min, float max)
{
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

bool SuperMech_CheckCollision_Player(const SuperMech *mech, Vector2 center, float radius)
{
    float mechWidth  = mech->frameWidth  * mech->scale;
    float mechHeight = mech->frameHeight * mech->scale;

    //mech rectangle top-left corner
    float rectX = mech->position.x;
    float rectY = mech->position.y;

    //find closest point on rectangle to circle center
    float closestX = ClampFloat(center.x, rectX, rectX + mechWidth);
    float closestY = ClampFloat(center.y, rectY, rectY + mechHeight);

    float dx = center.x - closestX;
    float dy = center.y - closestY;

    float distanceSquared = dx*dx + dy*dy;

    return distanceSquared <= radius * radius;
}

void SuperMech_Reset(SuperMech *mech, Vector2 startPos)
{
    mech->position = startPos;
    mech->velocity = (Vector2){0,0};

    mech->playerVisible = false;
    mech->stateTimer = 0.0f;

    mech->currentState = MECH_DORMANT;
    mech->previousState = MECH_DORMANT;

    mech->currentTexture = &mech->textureDormant;

    mech->currentFrame = 0;
    mech->animationTimer = 0.0f;
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

    switch (newState)
    {
        case MECH_DORMANT: mech->currentTexture = &mech->textureDormant; break;
        case MECH_IDLE:    mech->currentTexture = &mech->textureIdle;    break;
        case MECH_HUNT:    mech->currentTexture = &mech->textureHunt;    break;
        case MECH_SEARCH:  mech->currentTexture = &mech->textureSearch;  break;
        default:           mech->currentTexture = &mech->textureIdle;    break;
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

    mech->speedIdle = 60.0f;
    mech->speedHunt = 120.0f;
    mech->visionRange = 300.0f;

    mech->stateTimer = 0.0f;
    mech->lastKnownPlayerPos = startPos;
    mech->playerVisible = false;

    mech->currentState = MECH_DORMANT;
    mech->previousState = MECH_DORMANT;

    mech->textureDormant = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureIdle = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureHunt = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureSearch = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->currentTexture = &mech->textureDormant;
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

    if (mech->playerVisible)
    {
        mech->lastKnownPlayerPos = playerPos;
    }

    UpdateState(mech, dt);

    const char *stateName = SuperMech_GetStateName(mech->currentState);

    DrawText( stateName, (int)mech->position.x, (int)(mech->position.y - 20), 16, RED );
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

    DrawTexturePro( *mech->currentTexture, source, dest, origin, 0.0f, WHITE);
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
    if (mech->playerVisible)
    {
        ChangeState(mech, MECH_HUNT, dt);
    }
}

static void Dormant_Exit(SuperMech *mech, float dt)
{
}

//------------------Idle------------------//

static void Idle_Entry(SuperMech *mech, float dt)
{
    mech->stateTimer = 0.0f;
    mech->velocity = (Vector2){0,0};
}

static void Idle_Update(SuperMech *mech, float dt)
{
    mech->stateTimer += dt;

    if (mech->playerVisible)
    {
        ChangeState(mech, MECH_HUNT, dt);
        return;
    }

    if (mech->stateTimer > 5.0f)
    {
        ChangeState(mech, MECH_DORMANT, dt);
    }
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
    if (mech->playerVisible)
    {
        MoveTowards(mech, mech->lastKnownPlayerPos, mech->speedHunt, dt);
    }
    else
    {
        ChangeState(mech, MECH_SEARCH, dt);
    }    
}

static void Hunt_Exit(SuperMech *mech, float dt)
{
}

//------------------Search------------------//

static void Search_Entry(SuperMech *mech, float dt)
{
    mech->stateTimer = 0.0f;
    mech->velocity = (Vector2){0,0};
}

static void Search_Update(SuperMech *mech, float dt)
{
    mech->stateTimer += dt;

    if (mech->playerVisible)
    {
        ChangeState(mech, MECH_HUNT, dt);
        return;
    }

    if (mech->stateTimer < 1.0f)
    {
        mech->facingRight = true;
    }
    else if (mech->stateTimer < 2.0f)
    {
        mech->facingRight = false;
    }

    if (mech->stateTimer > 3.0f)
    {
        ChangeState(mech, MECH_IDLE, dt);
    }
}

static void Search_Exit(SuperMech *mech, float dt)
{
}