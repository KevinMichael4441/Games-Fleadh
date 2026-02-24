#include <math.h>

#include "supermech.h"

static void TryJump(SuperMech *mech, Vector2 target);
static bool AtLedge(SuperMech *mech, Vector2 target);
static void ApplyGravity(SuperMech *mech, float dt);

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

static void SetLevel(SuperMech *mech, LevelData* level)
{
    mech->currentLevel = *level;
}

static bool CanSeePlayer(const SuperMech *mech, Vector2 playerPos) 
{
    float dist = Vec2Distance(mech->position, playerPos);
    return dist <= mech->visionRange;
}

static void MoveTowards(SuperMech *mech, Vector2 target, float speed, float dt)
{
    float dx = target.x - mech->position.x;
    //TraceLog(LOG_INFO, "target: %d", target.x);

    if(!AtLedge(mech, target))
    {
        if (fabsf(dx) > 5.0f)
        {
            float dir = (dx > 0) ? 1.0f : -1.0f;

            mech->velocity.x = dir * speed;

            mech->facingRight = (dir > 0);
        }
        else
        {
            mech->velocity.x = 0;
        }
    }

    TryJump(mech, target);
}

static void TryJump(SuperMech *mech, Vector2 target)
{
    float xDist = fabsf(target.x - mech->position.x);
    float yDiff = mech->position.y - target.y;

    if ((yDiff > 40.0f) && (xDist < 120.0f) && mech->isGrounded)
    {
        mech->velocity.y = -mech->jumpForce;
        mech->isGrounded = false;
    }
}

static bool AtLedge(SuperMech *mech, Vector2 target)
{
    return false;
}

static void ApplyGravity(SuperMech *mech, float dt)
{
    if (!mech->isGrounded)
    {
        mech->velocity.y += mech->gravity * dt;
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

static c2AABB SuperMech_GetAABB(const SuperMech* mech)
{
    c2AABB box;

    box.min = c2V(mech->position.x, mech->position.y);
    box.max = c2V( mech->position.x + mech->frameWidth, mech->position.y + mech->frameHeight );

    return box;
}

static int SuperMech_FindBoundaryAABBs( const SuperMech* mech, c2AABB outRects[MAX_BOUNDARY_RECTS])
{
    if (!&mech->currentLevel) return 0;

    Vector2 center = { mech->position.x + mech->frameWidth * 0.5f, mech->position.y + mech->frameHeight * 0.5f };

    int tileX = (int)(center.x / mech->currentLevel.tileWidth);
    int tileY = (int)(center.y / mech->currentLevel.tileHeight);

    int count = 0;

    for (int oy = -2; oy <= 2; oy++)
    {
        for (int ox = -2; ox <= 2; ox++)
        {
            int checkTileX = tileX + ox;
            int checkTileY = tileY + oy;

            float tileCenterX = checkTileX * mech->currentLevel.tileWidth + mech->currentLevel.tileWidth * 0.5f;
            float tileCenterY = checkTileY * mech->currentLevel.tileHeight + mech->currentLevel.tileHeight * 0.5f;

            if (Level_IsBoundaryPos(&mech->currentLevel, tileCenterX, tileCenterY))
            {
                c2AABB tile;

                tile.min = c2V( checkTileX * mech->currentLevel.tileWidth, checkTileY * mech->currentLevel.tileHeight);
                tile.max = c2V( tile.min.x + mech->currentLevel.tileWidth, tile.min.y + mech->currentLevel.tileHeight);

                outRects[count++] = tile;

                if (count >= MAX_BOUNDARY_RECTS)
                    return count;
            }
        }
    }

    return count;
}

static void SuperMech_ResolveVsTile( SuperMech* mech, const c2AABB* tile)
{
    c2AABB mechBox = SuperMech_GetAABB(mech);

    c2Manifold manifold = {};
    c2AABBtoAABBManifold(mechBox, *tile, &manifold);

    if (manifold.count == 0) return;

    float push = manifold.depths[0];

    mech->position.x -= manifold.n.x * push;
    mech->position.y -= manifold.n.y * push;

    mechBox = SuperMech_GetAABB(mech);

    float mechBottom = mechBox.max.y;
    float mechTop = mechBox.min.y;

    float tileTop = tile->min.y;
    float tileBottom = tile->max.y;

    const float epsilon = 0.5f;

    if (fabsf(mechBottom - tileTop) < epsilon && mech->velocity.y >= 0)
    {
        mech->velocity.y = 0;
        mech->isGrounded = true;
    }
    else if (fabsf(mechTop - tileBottom) < epsilon && mech->velocity.y <= 0)
    {
        mech->velocity.y = 0;
    }
}

void SuperMech_ResolveBoundaryCollision(SuperMech* mech)
{
    mech->isGrounded = false;
    const int iterations = 4;

    for (int i = 0; i < iterations; i++)
    {
        c2AABB tiles[MAX_BOUNDARY_RECTS];
        int count = SuperMech_FindBoundaryAABBs(mech, tiles);

        for (int t = 0; t < count; t++)
        {
            SuperMech_ResolveVsTile(mech, &tiles[t]);
        }
    }
}

void SuperMech_Reset(SuperMech *mech, Vector2 startPos)
{
    mech->position = startPos;
    mech->velocity = (Vector2){0,0};
    mech->isGrounded = false;

    mech->playerVisible = false;

    mech->previousState = MECH_SEARCH;
    mech->currentState  = MECH_SEARCH;
    mech->stateTimer    = 0.0f;

    mech->currentTexture = &mech->textureSearch;
    mech->currentFrame = 0;
    mech->animationTimer = 0.0f;

    mech->facingRight = true;

    SuperMech_ResolveBoundaryCollision(mech);
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

void SuperMech_Init(SuperMech *mech, Vector2 startPos, LevelData* level) 
{
    SetLevel(mech, level);

    mech->position = startPos;
    mech->velocity = (Vector2){0,0};

    mech->speedIdle = 60.0f;
    mech->speedHunt = 120.0f;
    mech->visionRange = 300.0f;

    mech->gravity = 900.0f;
    mech->isGrounded = true;
    mech->jumpForce = 450.0f;

    mech->stateTimer = 0.0f;
    mech->lastKnownPlayerPos = startPos;
    mech->playerVisible = false;

    mech->currentState = MECH_SEARCH;
    mech->previousState = MECH_SEARCH;

    mech->textureDormant = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureIdle = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureHunt = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->textureSearch = LoadTexture("./assets/supermech/supermech_sleep_64x98.png");
    mech->currentTexture = &mech->textureSearch;
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

void SuperMech_Uppdate(SuperMech *mech, Vector2 playerPos, bool cameraTriggered, float dt) 
{
    mech->playerVisible = CanSeePlayer(mech, playerPos);

    if (cameraTriggered && mech->currentState != MECH_HUNT)
    {
        ChangeState(mech, MECH_HUNT, dt);
    }

    if (mech->playerVisible)
    {
        mech->lastKnownPlayerPos = playerPos;
    }

    mech->velocity.y += mech->gravity * dt;

    mech->position.x += mech->velocity.x * dt;
    mech->position.y += mech->velocity.y * dt;

    SuperMech_ResolveBoundaryCollision(mech);

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

    DrawTexturePro( *mech->currentTexture, source, dest, origin, 0.0f, WHITE);

    const char *stateName = SuperMech_GetStateName(mech->currentState);
    DrawText( stateName, (int)mech->position.x, (int)(mech->position.y - 20), 16, RED );
    DrawText(mech->isGrounded ? "GROUND" : "AIR", mech->position.x, mech->position.y - 40, 16, YELLOW);
    c2AABB box = SuperMech_GetAABB(mech);

    DrawRectangleLines( box.min.x, box.min.y, box.max.x - box.min.x, box.max.y - box.min.y, GREEN );
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