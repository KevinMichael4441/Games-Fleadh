#ifndef SECURITY_H
#define SECURITY_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "laserwall.h"
#include "security_camera.h"


class SecuritySystem
{
public:
	SecuritySystem();
	void initialize();
	void update(float t_dt);
	void draw();
	
private:

	Laserwall m_lasers[MAX_LASERWALL];
	SecurityCamera m_cameras[MAX_CAMERA];

};


#endif