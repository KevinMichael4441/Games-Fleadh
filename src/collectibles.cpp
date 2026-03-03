#include "collectibles.h"
#include <cstring>

//------------------Collectible------------------//

static Texture2D m_texture;
static bool m_textureLoaded;

Texture2D Collectible::m_texture = {};
bool Collectible::m_textureLoaded = false;

Collectible::Collectible()
{
    m_position = {0,0};
    m_radius = 8.0f;
    m_active = false;
}


void Collectible::Initialize(Vector2 pos, float radius)
{
    m_position = pos;
    m_radius = radius;
    m_active = true;
    m_bobTimer = 0.0f;

    if (!m_textureLoaded)
    {
        m_texture = LoadTexture("./assets/images/ENVIRONMENT/COIN.png");
        m_textureLoaded = true;
    }
}

void Collectible::Update(Ooze& player, int& score, float dt)
{
    if (!m_active) return;

    m_bobTimer += dt;

    const Point* points = player.GetPoints();
    int count = player.GetPointCount();

    for (int i = 0; i < count; i++)
    {
        float dx = points[i].m_position.x - m_position.x;
        float dy = points[i].m_position.y - m_position.y;

        float distSq = dx*dx + dy*dy;
        float playerRadius = (points[i].m_radiusX + points[i].m_radiusY) * 0.5f;
        float combined = playerRadius + m_radius;

        if (distSq <= combined * combined)
        {
            m_active = false;
            score += 1;
            break;
        }
    }
}

void Collectible::Draw() const
{
    if (!m_active) return;
    if (!m_textureLoaded || m_texture.id == 0)
    {
        return;
    }

    float bobOffset = 4.0f * sinf(m_bobTimer * 3.0f);
    Vector2 drawPos = { m_position.x - m_texture.width * 0.5f, m_position.y - m_texture.height * 0.5f + bobOffset };

    DrawTextureV(m_texture, drawPos, WHITE);
}

bool Collectible::IsActive() const
{
    return m_active;
}

//------------------Manager------------------//

Collectibles_Manager::Collectibles_Manager()
{
    m_collectibleCount = 0;
}

void Collectibles_Manager::Initialize(LevelData* level, float radius)
{
    m_collectibleCount = 0;

    if (!level || !level->objects || level->objectCount <= 0)
    {
        return;
    }

    for (int i = 0; i < level->objectCount; i++)
    {
        const LevelObject& obj = level->objects[i];

        if (!obj.type || strcmp(obj.type, "Collectible") != 0)
        {
            continue;
        }

        if (m_collectibleCount >= MAX_COLLECTIBLES)
        {
            break;
        }

        float radius = LevelObjectGetFloat(&obj, "radius", 8.0f);

        m_collectibles[m_collectibleCount].Initialize({ obj.x, obj.y }, radius);
        m_collectibleCount++;
    }
}

void Collectibles_Manager::Update(Ooze& player, int& score, float dt)
{
    for (int i = 0; i < m_collectibleCount; i++)
    {
        m_collectibles[i].Update(player, score, dt);
    }
}

void Collectibles_Manager::Draw() const
{
    for (int i = 0; i < m_collectibleCount; i++)
    {
        m_collectibles[i].Draw();
    }
}