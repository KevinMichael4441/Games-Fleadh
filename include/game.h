#ifndef GAME_H
#define GAME_H

#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include "constants.h"
#include "math.h"
#include "rlgl.h"

#ifdef __cplusplus
extern "C"
{
	#endif

	// GameData holds all the game related data
	// such as score, player info, level info etc.
	// Its passed to all game functions as a pointer
	// in order to read and modify the game state.
	// As its a pointer its efficient to pass around
	// But we must manage its memory properly to avoid memory leaks. 
	// Free GameData pointer on exit. See CloseGame()

	typedef struct Point
	{
		Vector2 m_acceleration;
		Vector2 m_velocity;
		Vector2 m_position;
		bool m_lock;

		float m_radius;
		float m_newRadius;
		float m_mass;
		Color m_color;

		bool collisionFlag;

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

	typedef struct GameData
	{
		Point points[MAX_POINTS];
		Spring springs[MAX_SPRINGS];

		Vector2 bezierPoints[MAX_BEZIER_POINTS];
		bool bezierMoving[MAX_BEZIER_POINTS];

		float springConstant;
		float speed;
		float jumpAmount;
		Vector2 gravity;

		float spawnTimer;
		float spawnDelay;
		bool pointsLocked;

		Vector2 pointCoords[MAX_POINTS + 1];
		Vector2 texCoords[MAX_POINTS + 1];

		float relapseTimer;
		bool inputRecieved;

		float mainRadius;
		float individualRadius;

		float bezierTimer;
		float bezierDelay;
		float currentAngle;
		int currentOffset;

		//Point masterPoint;
		float damp;
		float angleOffset;

		//float jumpTimer;
		//float jumpDelay;
		//bool isjumping;

		Vector2 centrePoint;

	} GameData;

	void InitGame(GameData *data);
	void UpdateGame(GameData *data, float deltaTime);

	void updatePoints(GameData *data, float deltaTime);
	void updateSprings(GameData *data, float deltaTime);
	void updateMasterPoint(GameData* data, float deltaTime);
	void updateBezierPoints(GameData* data, float deltaTime);
	void jump(GameData* data);
	void jumpActual(GameData* data);

	void makePointForBezierFromCircle(GameData* data);



	void updatePointForBezierFromCircle(GameData* data);


	void lockPoints(GameData *data);
	void unlockPoints(GameData *data);

	Vector2 calculateControlPoint(Vector2 t_m, Vector2 t_n, Vector2 centrePoint);

	Vector2 calculateCenter(GameData* data);

	void ClampPlayerOnScreen(GameData *data, float deltatime, int index);
	void DrawGame(GameData *data);
	void CloseGame(GameData *data);
	
#ifdef __cplusplus
}
#endif
#endif // GAME_H