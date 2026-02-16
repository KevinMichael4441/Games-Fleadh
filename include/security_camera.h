#ifndef CAMERA_H
#define CAMERA_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"

class SecurityCamera
{
public:
	SecurityCamera();
	void initialize(float t_x, float t_y);
	void update(float t_dt);
	void draw();


private:

	static const int m_width = 32;
	static const int m_height = 32;

	Rectangle m_body;
	Vector2 m_startPos;
	Vector2	m_endPos;

	bool m_goingLeft;
	int m_speed;

};


#endif