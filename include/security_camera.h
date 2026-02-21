#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"

class SecurityCamera
{
public:
	SecurityCamera();
	void initialize(float t_x, float t_y, float t_maxRotation, float t_minRotation);

	void update(float t_dt, Vector2 playerPos);
	void draw();
	
	bool isPlayerDetected() const;

private:
	static const int m_width = 32;
	static const int m_height = 32;

	Rectangle m_body;

	Vector2 m_origin;
    float m_angle;
    float m_maxAngle;
	float m_minAngle;

    float m_range;
	float m_speed; //radians per second

	bool m_isActive;
	float m_timer;
    float m_activeDuration;
    float m_inactiveDuration;

    bool m_playerDetected;
};

#endif