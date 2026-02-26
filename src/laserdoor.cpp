#include "laserdoor.h"

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

Vector2 Key::GetPosition() const
{
    return m_position;
}

void Key::Update(Ooze& player)
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
            break;
        }
    }
}

void Key::Draw() const
{
    if (!m_active)
        return;

    DrawCircleV(m_position, m_radius, GOLD);
}

//------------------LaserDoor------------------//

Laserdoor::Laserdoor()
{ 

}


void Laserdoor::initialize(float t_x, float t_y)
{
	m_boundingBox = {t_x, t_y, t_x + m_WIDTH, t_y + m_HEIGHT};

	for (int index = 0; index < MAX_BARS; index++)
	{
		m_bars[index] = {t_x + (10*index), t_y, m_BAR_WIDTH, m_BAR_HEIGHT};
	}

	m_timer = 0;
	m_delay = rand() % 5 + 8;
	m_isActive = true;
}


void Laserdoor::update(Ooze &t_ooze, float t_dt)
{
	m_timer += t_dt;

	if (m_timer > m_delay)
	{
		m_delay = rand() % 5 + 3;
		m_isActive = !m_isActive;
		m_timer = 0;
	}

	updateCollision(t_ooze);
}

void Laserdoor::updateCollision(Ooze &t_ooze)
{
	if (m_isActive)
	{
		Point *currentPoint;

		for (int index = 0; index < MAX_POINTS; index++)
		{
			currentPoint = &t_ooze.GetPoints()[index];
			c2Circle circle = {currentPoint->m_position.x, currentPoint->m_position.y, currentPoint->m_radiusX};

			if (c2CircletoAABB(circle, m_boundingBox))
			{
				c2Manifold manifold = {};	// create manifold object
	
				// is the circle and rec overlapping
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
}

void Laserdoor::draw()
{
	if(m_isActive)
	{
		DrawRectangle(m_boundingBox.min.x, m_boundingBox.min.y, m_WIDTH, m_HEIGHT, WHITE );
		
		for (int index = 0; index < MAX_BARS; index++)
		{
			DrawRectangleRec(m_bars[index], BLUE);
		}
	}
}