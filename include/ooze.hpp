#ifndef OOZE_H
#define OOZE_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "command.h"
#include "input_manager.h"
#include "fsm.h"


enum class Position{TOP, RIGHT, DOWN, LEFT, MIDDLE};
static const int MAX_COLLISION_PARTS = 4;

typedef struct Point
{
	Vector2 m_acceleration;
	Vector2 m_velocity;
	Vector2 m_position;

	float m_radiusX;		
	float m_newRadiusX;
	
	float m_radiusY;		
	float m_newRadiusY;
	
	float m_mass;

	float lerpTimeElapsedX;
	float lerpTimeElapsedY;
	float lerpTime;
	Position m_extremePos;
	
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

	float m_collisionTimer;

	bool collisionParts[MAX_COLLISION_PARTS];
public:
	Ooze();
	void Initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount);
	
	void HandleInput(Command t_activeCommand);
	void HandleEvent(Event t_event);

	void EnterState(State t_state);
    void EnterIdleState();
	void EnterMoveState();
	void EnterJumpState();
	void EnterCollideHorizontalState();
	void EnterCollideUpState();
	void EnterCollideDownState();
   


    void ExitState();
	void ExitIdleState();
	void ExitMoveState();
	void ExitJumpState();
	void ExitCollideHorizontalState();
	void ExitCollideUpState();
	void ExitCollideDownState();



	void Update(float t_dt, Command t_activeCommand);
	void DefaultUpdate(float t_dt);
	void UpdatePoints(float t_dt);
	void UpdateSprings();

	void UpdateState(float t_dt);
	void UpdateIdleState(float t_dt);
	void UpdateMoveState(float t_dt);
	void UpdateJumpState(float t_dt);
	void UpdateCollideHorizontalState(float t_dt);
	void UpdateCollideUpState(float t_dt);
	void UpdateCollideDownState(float t_dt);

	void ClampPointsOnScreen();
	void SetNewLerp(int index, int t_randX, int t_baseX, int t_randY, int t_baseY);

	void Move();
	void Jump();
	void Spread();

	Vector2 CalculateCenter();
	void calculateExtremePos();

	void Draw();


	
	FSM fsm;
};

#endif