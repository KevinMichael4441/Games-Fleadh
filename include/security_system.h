#ifndef SECURITY_H
#define SECURITY_H

#include <raylib.h>
#include <raymath.h>

#include "ooze.hpp"

#include "constants.h"
#include "laserwall.h"
#include "security_camera.h"
#include "level_loader.h"
#include <cstring>
#include "utility/camtype.h"

class SecuritySystem
{
public:
	SecuritySystem();
	void initialize(LevelData* t_level);
	
	bool update(float t_dt, Ooze &t_ooze);
	void draw();
	Laserwall m_lasers[MAX_LASERWALL];
	int m_laserCount;
	
private:
	SecurityCamera m_cameras[MAX_CAMERA];
	int m_cameraCount;
};

#endif