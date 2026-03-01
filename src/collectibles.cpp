#include "collectibles.h"

//------------------Collectible------------------//

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
    m_texture = LoadTexture("./assets/images/ENVIRONMENT/COIN.png");
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

        float playerRadius =
            (points[i].m_radiusX + points[i].m_radiusY) * 0.5f;

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
}

void Collectibles_Manager::Initialize(Vector2 pos, float radius)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++)
    {
        m_collectibles[i].Initialize({pos.x + 30 * i, pos.y}, radius);
    }
}

void Collectibles_Manager::Update(Ooze& player, int& score, float dt)
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++)
    {
        m_collectibles[i].Update(player, score, dt);
    }
}

void Collectibles_Manager::Draw() const
{
    for (int i = 0; i < MAX_COLLECTIBLES; i++)
    {
        m_collectibles[i].Draw();
    }
}