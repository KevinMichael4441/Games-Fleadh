#ifndef OOZE_H
#define OOZE_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "command.h"
#include "input_manager.h"

#include "level_loader.h"
#include "fsm.h"

extern "C" {
#include "cute_c2.h"
}

static const int MAX_COLLISION_PARTS = 4;

enum class SQUISH_AMOUNT{HIGH, MEDIUM, LOW, NOP};

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

	int id;
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
	SQUISH_AMOUNT m_squishiness;

	LevelData* m_level = nullptr;
	static const int MAX_BOUNDARY_RECTS = 16;


	float m_toCollideTimer = 0.0f;
	const float m_toCollideDelay = 0.3f;

public:
	Ooze();
	void Initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount);
	
	void HandleInput(Command t_activeCommand);
	bool HandleEvent(Event t_event);

	void EnterState(State t_state, Event t_event);
    void EnterIdleState();
	void EnterMoveState();
	void EnterJumpState();
	void EnterCollideHorizontalState();
	void EnterCollideVerticalState();
   
    void ExitState();
	void ExitIdleState();
	void ExitMoveState();
	void ExitJumpState();
	void ExitCollideState();

	void Update(float t_dt, Command t_activeCommand);
	void DefaultUpdate(float t_dt);
	void UpdatePoints(float t_dt);
	void UpdateSprings();

	void UpdateState(float t_dt);
	void UpdateIdleState(float t_dt);
	void UpdateMoveState(float t_dt);
	void UpdateJumpState(float t_dt);
	void UpdateCollideHorizontalState(float t_dt);
	void UpdateCollideVerticalState(float t_dt);

	void SetNewLerp(int index, int t_randX, int t_baseX, int t_randY, int t_baseY);

	void Move();
	void Jump();
	void Spread();

	void LowHorizontalCollisionAnimation();
	void MediumHorizontalCollisionAnimation();
	void HighHorizontalCollisionAnimation();

	void LowVerticalCollisionAnimation();
	void MediumVerticalCollisionAnimation();
	void HighVerticalCollisionAnimation();

	Vector2 CalculateCenter();

	void Draw();

	void SetLevel(LevelData* level);
	int FindBoundaryAABBs(Vector2 centerPos, c2AABB outRects[MAX_BOUNDARY_RECTS]) const;
	void ResolvePointVsAABB(Point& p, const c2AABB& rec, float slop, float str, float friction);
	void Reset(Vector2 startPos);

	Vector2 getPosition();
	Point* GetPoints();
	int GetPointCount() const;
	
	FSM fsm;
};

#endif