#ifndef CAMERA_H
#define CAMERA_H

#include <raylib.h>
#include <raymath.h>
#include <cute_c2.h>
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
	void initialize(float t_x, float t_y, float t_maxRotation, float t_minRotation);

	void update(float t_dt, Vector2 playerPos);
	void draw();

	void initRaycast();
	void updateRaycast();
	void drawRaycast();
	
	bool isPlayerDetected() const;
	void SetLevel(LevelData* level);

private:
	static const int m_width = 32;
	static const int m_height = 32;

	Rectangle m_body;

	Vector2 m_origin;
	Vector2 m_visualEndPoint;
	Vector2 m_actualEndPoint;

	c2Ray m_laser;

	const float m_range = 600;

    float m_angle;
    float m_maxAngle;
	float m_minAngle;

	float m_speed; //radians per second

	bool m_isActive;
	float m_timer;
    float m_activeDuration;
    float m_inactiveDuration;

    bool m_playerDetected;


	LevelData* m_level = nullptr;

	Ray m_ray;
	//std::vector<Intersection> m_intersections;

	bool camCheckCollisionPlayer(Vector2 t_center);
	void FindBoundaryAABBs(double x0, double y0, double x1, double y1);	

	// void findIntersection(Vector2 t_start, Vector2 t_end);
	// void findIntersections(c2AABB t_line);
	// void findClosestIntersection();
};

#endif