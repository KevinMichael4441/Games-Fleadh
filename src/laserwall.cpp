#include "laserwall.h"


Laserwall::Laserwall()
{ 

}


void Laserwall::initialize(float t_x, float t_y)
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


void Laserwall::update(Ooze &t_ooze, float t_dt)
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

void Laserwall::updateCollision(Ooze &t_ooze)
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


			if (m_isActive)
			{
				if (manifold.n.x > 0.1)
				{
					currentPoint->m_velocity.x -= WALL_PUSHBACK;
				}
				else if(manifold.n.x < -0.1)
				{
					currentPoint->m_velocity.x += WALL_PUSHBACK;
				}
			}
			else
			{
				if (manifold.n.x > 0.1)
				{
					currentPoint->m_velocity.x -= manifold.depths[0];
				}
				else if(manifold.n.x < -0.1)
				{
					currentPoint->m_velocity.x += manifold.depths[0];
				}
			}

		}
	}
}

void Laserwall::draw()
{
	DrawRectangle(m_boundingBox.min.x, m_boundingBox.min.y, m_WIDTH, m_HEIGHT, WHITE );
	if(m_isActive)
	{
		for (int index = 0; index < MAX_BARS; index++)
		{
			DrawRectangleRec(m_bars[index], RED);
		}
	}

}