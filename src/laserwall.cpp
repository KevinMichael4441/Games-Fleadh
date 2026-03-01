#include "laserwall.h"

Laserwall::Laserwall()
{ 

}

void Laserwall::initialize(float t_x, float t_y)
{
	m_texture = LoadTexture("./assets/images/ENVIRONMENT/LASER.png");

    m_currentFrame = 7;
	m_frameWidth = 64;
	m_frameHeight = 98;
    m_targetActive = true;

    m_boundingBox = {t_x, t_y, t_x + m_WIDTH, t_y + m_HEIGHT};

    for (int index = 0; index < MAX_BARS; index++)
    {
        m_bars[index] = {t_x + (10*index), t_y, m_BAR_WIDTH, m_BAR_HEIGHT};
    }

    m_timer = 0;
    m_delay = rand() % 5 + 8;
    m_isActive = true;
}


bool Laserwall::update(Ooze &t_ooze, float t_dt)
{
	m_timer += t_dt;

    if (m_timer > m_delay)
    {
        m_delay = rand() % 5 + 3;

        m_targetActive = !m_targetActive;
        m_isActive = m_targetActive;

        m_timer = 0;
    }

    m_animTimer += t_dt;

	if (m_animTimer >= m_animSpeed)
    {
        m_animTimer = 0;

        if (m_targetActive)
        {
            if (m_currentFrame < 7)
                m_currentFrame++;
        }
        else
        {
            if (m_currentFrame > 0)
                m_currentFrame--;
        }
	}

    return updateCollision(t_ooze);

}

bool Laserwall::updateCollision(Ooze &t_ooze)
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

					return true;
			}	
		}
	}

	return false;
}

void Laserwall::draw()
{
    if (!m_texture.id) return;

    Rectangle source = { (float)(m_currentFrame * m_frameWidth), 0, (float)m_frameWidth, (float)m_frameHeight };
    Vector2 position = { m_boundingBox.min.x, m_boundingBox.min.y };
    DrawTextureRec(m_texture, source, position, WHITE);
}