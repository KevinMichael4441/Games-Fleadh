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
	float m_moveDirection;

	State m_currentState;

public:
	Ooze();
	void Initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount);
	
	void HandleInput(Command t_activeCommand);
	void HandleEvent(Event t_event);

	void EnterState(State t_state);
    void EnterIdleState();
	void EnterMoveState();
	void EnterJumpState();
   


    void ExitState();
	void ExitIdleState();
	void ExitMoveState();
	void ExitJumpState();



	void Update(float t_dt, Command t_activeCommand);
	void DefaultUpdate(float t_dt);
	void UpdatePoints(float t_dt);
	void UpdateSprings();

	void UpdateState(float t_dt);
	void UpdateIdleState(float t_dt);
	void UpdateMovingState(float t_dt);
	void UpdateJumpingState(float t_dt);
	



	void ClampPlayerOnScreen(int index);

	void Move();
	void Jump();
	void Spread();

	Vector2 CalculateCenter();

	void Draw();


	
	FSM fsm;
};

#endif