#ifndef SECURITY_H
#define SECURITY_H

#include <raylib.h>
#include <raymath.h>

#include "ooze.hpp"

#include "constants.h"
#include "laserwall.h"
#include "security_camera.h"



class SecuritySystem
{
public:
	SecuritySystem();
	void initialize(LevelData* t_level);
	
	bool update(float t_dt, Ooze &t_ooze);
	void draw();
	
private:
	Laserwall m_lasers[MAX_LASERWALL];
	SecurityCamera m_cameras[MAX_CAMERA];
};

#endif