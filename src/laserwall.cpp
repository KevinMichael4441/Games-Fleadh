#include "laserwall.h"


Laserwall::Laserwall()
{ 

}


void Laserwall::initialize(float t_x, float t_y)
{
	m_boundingBox = {t_x, t_y, m_WIDTH, m_HEIGHT};

	for (int index = 0; index < MAX_BARS; index++)
	{
		m_bars[index] = {t_x + (10*index), t_y, m_BAR_WIDTH, m_BAR_HEIGHT};
	}

	m_timer = 0;
	m_delay = rand() % 5 + 8;
	m_isActive = rand() % 2;
}


void Laserwall::update(float t_dt)
{
	m_timer += t_dt;

	if (m_timer > m_delay)
	{
		m_delay = rand() % 5 + 3;
		m_isActive = !m_isActive;
		m_timer = 0;
	}

}

void Laserwall::draw()
{
	if (m_isActive)
	{
		for (int index = 0; index < MAX_BARS; index++)
		{
			DrawRectangleRec(m_bars[index], RED);
		}
	}

}