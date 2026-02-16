#include "security_camera.h"


SecurityCamera::SecurityCamera()
{ 

}


void SecurityCamera::initialize(float t_x, float t_y)
{
	m_body = {t_x, t_y, m_width, m_height};

	m_startPos = {t_x + (m_width / 2), (t_y + m_height/2)};

	m_endPos = {0, SCREEN_WIDTH};

	m_goingLeft = false;

	m_speed =  rand() % 5 + 100;
}


void SecurityCamera::update(float t_dt)
{
	if (m_goingLeft)
	{
		m_endPos.x -= (t_dt * m_speed);
	}
	else
	{
		m_endPos.x += (t_dt * m_speed);
	}
	

	if (m_endPos.x < 0)
	{
		m_endPos.x = 0;
		m_goingLeft = false;
		m_speed =  rand() % 5 + 100;
	}
	else if (m_endPos.x > SCREEN_WIDTH)
	{
		m_endPos.x = SCREEN_WIDTH;
		m_goingLeft	= true;
		m_speed =  rand() % 5 + 100;
	}
}

void SecurityCamera::draw()
{
	DrawRectangleRec(m_body, ORANGE);

	DrawLineV(m_startPos, m_endPos, RED);
}