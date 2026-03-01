#ifndef SECURITY_H
#define SECURITY_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "laserwall.h"
#include "security_camera.h"
#include "level_loader.h"
#include <cstring>

class SecuritySystem
{
public:
	SecuritySystem();
	void initialize(LevelData* t_level);
	
	bool update(float t_dt, Vector2 playerPos);
	void draw();
	
private:
	Laserwall m_lasers[MAX_LASERWALL];
	SecurityCamera m_cameras[MAX_CAMERA];
	int m_cameraCount;
};

#endif