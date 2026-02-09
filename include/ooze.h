#ifndef OOZE_H
#define OOZE_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"


typedef struct Point
{
	Vector2 m_acceleration;
	Vector2 m_velocity;
	Vector2 m_position;

	float m_radius;		
	float m_newRadius;
	float m_mass;

	float lerpTimeElapsed;
	float lerpTime;
} Point;


typedef struct Spring
{
	float springConstant;
	float restLength;

	Point *a;
	Point *b;

} Spring;


class Ooze
{
private:
	float m_springConstant;

	Point m_points[MAX_POINTS];
	Spring m_springs[MAX_SPRINGS];

	float m_speed;
	float m_jumpAmount;
	Vector2 m_gravity;

	float m_mainRadius;
	float m_individualRadius;

	float m_damp;

	Vector2 m_centrePoint;

public:
	Ooze();
	void initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount);
	
	void update(float t_dt);
	void updatePoints(float t_dt);
	void updateSprings();

	void clampPlayerOnScreen(int index);

	void jump();
	void spread();

	Vector2 calculateCenter();


	void draw();
};


#endif