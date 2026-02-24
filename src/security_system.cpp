#include "security_system.h"

SecuritySystem::SecuritySystem()
{}

void SecuritySystem::initialize(LevelData *t_level)
{
	for(int index = 0; index < MAX_CAMERA; index++)
	{
		m_cameras[index].initialize(192, 128, 40, 120);
		m_cameras[index].SetLevel(t_level);
	}

	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].initialize(500, SCREEN_HEIGHT - 300);
	}
}


bool SecuritySystem::update(float t_dt, Ooze &t_ooze)
{
	bool detected = false;

	for(int i = 0; i < MAX_CAMERA; i++)
    {
        m_cameras[i].update(t_dt, t_ooze.CalculateCenter());

        if (m_cameras[i].isPlayerDetected())
        {
            detected = true;
        }
    }

	for (int i = 0; i < MAX_LASERWALL; i++)
    {
        m_lasers[i].update(t_ooze, t_dt);
    }

    return detected;
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