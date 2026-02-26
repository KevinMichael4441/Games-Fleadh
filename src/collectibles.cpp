#include "collectibles.h"

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
}

bool Collectible::IsActive() const
{
    return m_active;
}

Vector2 Collectible::GetPosition() const
{
    return m_position;
}

float Collectible::GetRadius() const
{
    return m_radius;
}

void Collectible::Update(Ooze& player, int& score)
{
    if (!m_active)
        return;

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
    if (!m_active)
        return;

    DrawCircleV(m_position, m_radius, GOLD);
}

//------------------Manager------------------//

Collectibles_Manager::Collectibles_Manager()
{
    m_count = 0;
}

void Collectibles_Manager::AddCollectible(Vector2 pos, float radius)
{
    if (m_count >= MAX_COLLECTIBLES)
        return;

    m_collectibles[m_count].Initialize(pos, radius);
    m_count++;
}

void Collectibles_Manager::Update(Ooze& player, int& score)
{
    for (int i = 0; i < m_count; i++)
    {
        m_collectibles[i].Update(player, score);
    }
}

void Collectibles_Manager::Draw() const
{
    for (int i = 0; i < m_count; i++)
    {
        m_collectibles[i].Draw();
    }
}

void Collectibles_Manager::Reset()
{
    m_count = 0;
}