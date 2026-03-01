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
#include "utility/camtype.h"

typedef struct 
{
	Vector2 m_pointOfIntersection;
	float m_T1;
}Intersection;

class SecurityCamera
{
public:
	SecurityCamera();
	void initialize(float t_x, float t_y, float t_distance, CamType t_type, CamMount t_mount, LaserDir t_dir);

	void update(float t_dt, Vector2 playerPos);
	void draw();
	void Frame_Update(float dt);
	void Animate();

	void drawRaycast();
	bool raycastPlayerCollision(Vector2& t_center);

	
	bool isPlayerDetected() const;
	void SetLevel(LevelData* level);

private:
	//animation
	Rectangle m_body;

	float m_sourceY;
	Vector2 m_position;
	Texture2D m_texture;
	int m_frameWidth;
    int m_frameHeight;
    float m_scale;

    int m_frameCount;
    float m_frameTime;
    int m_currentFrame;
    float m_animationTimer;

	int m_targetFrame = 0;
	bool m_animating = false;
	bool m_previousActive = false;

	//cam type
	CamType m_type{CAM_NONE};

	//ray
	float m_minX;
	float m_maxX;
	float m_yLevel;
	static const int WIDTH = 32;
	static const int HEIGHT = 32;
	float MAX_ANGLE = 0.75f;
	float MIN_ANGLE = -0.75f;

	c2Ray m_laser;
	Vector2 m_origin;
	Vector2 m_end;
	Vector2 m_direction;
	float m_length;
	float m_angle;

	float angleV; // Velocity
	float extendSpd{3.0f};

	//detection
	bool m_isActive;
    bool m_playerDetected;
	bool m_movingRight;
	float m_activeDuration;
	float m_inactiveDuration;
	float m_timer;

	LevelData* m_level = nullptr;
};

#endif