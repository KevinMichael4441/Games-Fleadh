#include "laserdoor.h"

//------------------LaserDoorManager------------------//

LaserDoor_Manager::LaserDoor_Manager()
{
}

void LaserDoor_Manager::Initialize(Vector2 lazer_pos, Vector2 key_pos, float key_radius)
{
	for(int i = 0; i < MAX_PAIRS; i++)
	{
		//m_laserdoor_pairs[i].first.Initialize({key_pos.x + 160 * i, key_pos.y}, key_radius);
		//m_laserdoor_pairs[i].second.Initialize({lazer_pos.x + 160 * i, lazer_pos.y});

		m_laserdoor_pairs[i].first.Initialize({key_pos.x, key_pos.y}, key_radius);
		m_laserdoor_pairs[i].second.Initialize({lazer_pos.x, lazer_pos.y});
	}
}

void LaserDoor_Manager::Update(Ooze& player, float dt)
{
	for(int i = 0; i < MAX_PAIRS; i++)
	{
		if(m_laserdoor_pairs[i].first.Update(player))
		{
			m_laserdoor_pairs[i].second.Disactivate();
		}

		m_laserdoor_pairs[i].second.Update(player, dt);
	}
}

void LaserDoor_Manager::Draw() const
{
	for(int i = 0; i < MAX_PAIRS; i++)
	{
		m_laserdoor_pairs[i].first.Draw();
		m_laserdoor_pairs[i].second.Draw();
	}
}

//------------------Key------------------//

Key::Key()
{
    m_position = {0,0};
    m_radius = 8.0f;
    m_active = false;
}

void Key::Initialize(Vector2 pos, float radius)
{
    m_position = pos;
    m_radius = radius;
    m_active = true;
}

bool Key::Update(Ooze& player)
{
    if (!m_active) return false;

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
			return true;
        }
    }

	return false;
}

void Key::Draw() const
{
    if (!m_active) return;

    DrawCircleV(m_position, m_radius, PINK);
}

//------------------LaserDoor------------------//

LaserDoor::LaserDoor()
{
	m_active = false;
	m_boundingBox = {0, 0, 0 + m_WIDTH, 0 + m_HEIGHT};
}


void LaserDoor::Initialize(Vector2 pos)
{
	m_boundingBox = {pos.x, pos.y, pos.x + m_WIDTH, pos.y + m_HEIGHT};

	for (int index = 0; index < MAX_BARS; index++)
	{
		m_bars[index] = {pos.x + (10*index), pos.y, m_BAR_WIDTH, m_BAR_HEIGHT};
	}

	m_active = true;

	m_texture = LoadTexture("./assets/images/ENVIRONMENT/LASER_DOOR.png");
	m_currentFrame = 13;
	m_targetFrame = 13;
	m_animating = false;
	m_animTimer = 0.0f;
	m_animSpeed = 0.08f;
}

void LaserDoor::Update(Ooze &player, float dt)
{
	if (m_active)
	{
		Point *currentPoint;

		for (int index = 0; index < MAX_POINTS; index++)
		{
			currentPoint = &player.GetPoints()[index];
			c2Circle circle = {currentPoint->m_position.x, currentPoint->m_position.y, currentPoint->m_radiusX};

			if (c2CircletoAABB(circle, m_boundingBox))
			{
				c2Manifold manifold = {};
 	 			c2CircletoAABBManifold(circle, m_boundingBox, &manifold);

				if (manifold.n.x > 0.1)
				{
					currentPoint->m_position.x -= (manifold.n.x * manifold.depths[0]);
					currentPoint->m_velocity.x -= WALL_PUSHBACK;
				}
				else if(manifold.n.x < -0.1)
				{
					currentPoint->m_velocity.x += WALL_PUSHBACK;
					currentPoint->m_position.x += (manifold.n.x * manifold.depths[0]);
				}

				break;
			}	
		}
	}

	if (m_animating)
	{
		m_animTimer += dt;
		if (m_animTimer >= m_animSpeed)
		{
			m_animTimer = 0.0f;

			if (m_currentFrame > m_targetFrame)
			{
				m_currentFrame--;
			}
			else
			{
				m_animating = false;
				m_currentFrame = m_targetFrame;
			}
		}
	}
}

void LaserDoor::Disactivate()
{
	if (m_active)
	{
		m_active = false;
		m_targetFrame = 1;
		m_animating = true;
	}
}

void LaserDoor::Draw() const
{
	if (!m_texture.id) return;

	Rectangle source = { (float)(m_currentFrame * 64), 0, 64, 96 };
	Vector2 position = { m_boundingBox.min.x, m_boundingBox.min.y };

	DrawTextureRec(m_texture, source, position, WHITE);
}