#ifndef OOZE_H
#define OOZE_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "command.h"
#include "input_manager.h"
#include "fsm.h"


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

	Command m_activeCommand;

public:
	Ooze();
	void Initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount);
	
	void HandleInput();
	void Update(float t_dt);
	void UpdatePoints(float t_dt);
	void UpdateSprings();

	void ClampPlayerOnScreen(int index);

	void Jump();
	void Spread();

	Vector2 CalculateCenter();


	void Draw();

	FSM slimeFSM;
};

#endif