#include "security_system.h"

SecuritySystem::SecuritySystem()
{}

void SecuritySystem::initialize(LevelData *t_level)
{
	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].initialize(450, SCREEN_HEIGHT - 128);
	}
	
	m_cameras[0].initialize(400.0f, 150.0f, 200.0f, CAM_SPOT, SE);
	m_cameras[1].initialize(300.0f, 150.0f, 200.0f, CAM_SPOT, NW);
}


bool SecuritySystem::update(float t_dt, Ooze &t_ooze)
{
	bool detected = false;

	for (int i = 0; i < MAX_LASERWALL; i++)
    {
        m_lasers[i].update(t_ooze, t_dt);
    }

	for(int i = 0; i < MAX_CAMERA; i++)
	{
		m_cameras[i].update(t_dt, t_ooze.CalculateCenter());
	}

    return detected;
}

void SecuritySystem::draw()
{
	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].draw();
	}
	for(int i = 0; i < MAX_CAMERA; i++)
	{
		m_cameras[i].draw();
	}

}