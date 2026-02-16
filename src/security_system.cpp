#include "security_system.h"

SecuritySystem::SecuritySystem()
{}

void SecuritySystem::initialize()
{
	for(int index = 0; index < MAX_CAMERA; index++)
	{
		m_cameras[index].initialize(200,120);
	}

	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].initialize(500, SCREEN_HEIGHT - 128);
	}
}


void SecuritySystem::update(float t_dt)
{
	for(int index = 0; index < MAX_CAMERA; index++)
	{
		m_cameras[index].update(t_dt);
	}

	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].update(t_dt);
	}

}

void SecuritySystem::draw()
{
	for(int index = 0; index < MAX_CAMERA; index++)
	{
		m_cameras[index].draw();
	}

	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].draw();
	}

}