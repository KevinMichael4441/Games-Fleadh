#include "security_system.h"

SecuritySystem::SecuritySystem()
{
}

void SecuritySystem::initialize(LevelData *t_level)
{
	m_cameraCount = 0;
    //m_laserCount = 0;

    if (!t_level || !t_level->objects || t_level->objectCount <= 0)
    {
        return;
    }

    for (int i = 0; i < t_level->objectCount; i++)
    {
        const LevelObject& obj = t_level->objects[i];

        //SECURITY CAMERA INIT
        if (obj.type && strcmp(obj.type, "SecurityCamera") == 0)
        {
            if (m_cameraCount >= MAX_CAMERA) continue;

            CamType type = (CamType)LevelObjectGetInt(&obj, "CamType", CAM_SPOT);
            float distance = LevelObjectGetFloat(&obj, "distance", 200.0f);
            //distance = 200.0f;
            CamMount mount = (CamMount)LevelObjectGetInt(&obj, "CamMount", MOUNT_BACKGROUND);
            LaserDir dir = (LaserDir)LevelObjectGetInt(&obj, "laserDir", LASER_S);
            //dir = LASER_E;

            m_cameras[m_cameraCount].initialize(obj.x, obj.y, distance, type, mount, dir);

            m_cameraCount++;
        }
        //LASER WALL INIT
        //else if (obj.type && strcmp(obj.type, "Laserwall") == 0)
        //{
        //    if (m_laserCount >= MAX_LASERWALL)
        //        continue;

        //    m_lasers[m_laserCount].initialize(obj.x, obj.y);

        //    m_laserCount++;
        //}
    }
}

bool SecuritySystem::update(float t_dt, Ooze &t_ooze)
{
	bool detected = false;

	//for (int i = 0; i < m_laserCount; i++)
    //{
    //    m_lasers[i].update(t_ooze, t_dt);
    //}

	for(int i = 0; i < m_cameraCount; i++)
	{
		m_cameras[i].update(t_dt, t_ooze.CalculateCenter());
        if (m_cameras[i].isPlayerDetected())
        {
            detected = true;
        }
	}

    return detected;
}

void SecuritySystem::draw()
{
    //for (int index = 0; index < m_laserCount; index++)
	//{
	//	m_lasers[index].draw();
	//}
    
	for(int index = 0; index < m_cameraCount; index++)
	{
		m_cameras[index].draw();
	}
}