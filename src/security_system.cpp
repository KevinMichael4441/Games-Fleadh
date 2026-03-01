#include "security_system.h"

SecuritySystem::SecuritySystem()
{}

bool SecuritySystem::update(float t_dt, Vector2 playerPos)
{
	bool detected = false;

	for(int i = 0; i < m_cameraCount; i++)
    {
        m_cameras[i].update(t_dt, playerPos);

        if (m_cameras[i].isPlayerDetected())
        {
            detected = true;
        }
    }

	for (int i = 0; i < MAX_LASERWALL; i++)
    {
        m_lasers[i].update(t_dt);
    }

    return detected;
}

void SecuritySystem::draw()
{
	for(int index = 0; index < m_cameraCount; index++)
	{
		m_cameras[index].draw();
	}

	for (int index = 0; index < MAX_LASERWALL; index++)
	{
		m_lasers[index].draw();
	}

}

void SecuritySystem::initialize(LevelData* t_level)
{
    m_cameraCount = 0;

    if (!t_level || !t_level->objects || t_level->objectCount <= 0)
    {
        return;
    }

    for (int i = 0; i < t_level->objectCount; i++)
    {
        const LevelObject& obj = t_level->objects[i];

        if (!obj.type || strcmp(obj.type, "SecurityCamera") != 0)
        {
            continue;
        }

        if (m_cameraCount >= MAX_CAMERA)
        {
            break;
        }

        CamType type = (CamType)LevelObjectGetInt(&obj, "CamType", CAM_SPOT);
        float distance = LevelObjectGetFloat(&obj, "distance", 200.0f);
        CamMount mount = (CamMount)LevelObjectGetInt(&obj, "CamMount", MOUNT_BACKGROUND);
        LaserDir dir = (LaserDir)LevelObjectGetInt(&obj, "laserDir", LASER_S);
        m_cameras[m_cameraCount].initialize(obj.x, obj.y, distance, type, mount, dir);

        m_cameraCount++;
    }
}