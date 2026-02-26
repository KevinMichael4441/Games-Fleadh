#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <raymath.h>
#include <cstdio>
extern "C" {
#include "cute_c2.h"
}
#include <vector>
#include <limits> // for infinity

#include "constants.h"
#include "level_loader.h"


typedef struct 
{
	Vector2 m_pointOfIntersection;
	float m_T1;
}Intersection;

class SecurityCamera
{
public:
	SecurityCamera();
	void initialize(float t_x, float t_y);

	void update(float t_dt, Vector2 playerPos);
	void draw();

	void drawRaycast();
	bool raycastPlayerCollision(Vector2& t_center);

	
	bool isPlayerDetected() const;
	void SetLevel(LevelData* level);

private:
	Rectangle m_body;
	static const int WIDTH = 32;
	static const int HEIGHT = 32;
	const float MAX_ANGLE = 0.75f;
	const float MIN_ANGLE = -0.75f;


	c2Ray m_laser;
	Vector2 m_origin;
	Vector2 m_end;
	Vector2 m_direction;
	float m_length;
	const float MIN_LENGTH = 300.0f;
	const float MAX_LENGTH = 500.0f;
	float m_angle;

	float angleV; // Velocity
	float extendSpd{3};

	bool m_isActive;
    bool m_playerDetected;
	bool m_movingRight;

	LevelData* m_level = nullptr;
};

#endif