#include "security_camera.h"

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd);

SecurityCamera::SecurityCamera()
{ 
}


void SecurityCamera::initialize(float t_x, float t_y, float t_distance, CamType t_type, CamDirection t_direction)
{
    m_type = t_type;
    m_fix = t_direction;
    m_origin = { t_x + WIDTH / 2.0f, t_y + HEIGHT / 2.0f };
    m_length = t_distance;

    switch(m_fix){
        case N:
            m_angle = 3.125f;
            MAX_ANGLE = 3.85f;
	        MIN_ANGLE = 2.35f;
        break;
        case S:
            m_angle = 0.625f;
            MAX_ANGLE = 0.75f;
	        MIN_ANGLE = -0.75f;
        break;
        case E:
            m_angle = 1.875f;
            MAX_ANGLE = 2.40f;
	        MIN_ANGLE = 0.825f;
        break;
        case W:
            m_angle = -1.875f;
            MAX_ANGLE = -0.825f;
	        MIN_ANGLE = -2.40f;
        break;
        case NE:
            m_angle = 2.5f;
            MAX_ANGLE = 3.125f;
	        MIN_ANGLE = 1.575f;
        break;
        case NW:
            m_angle = 3.75f;
            MAX_ANGLE = 4.725f;
	        MIN_ANGLE = 3.125f;
        break;
        case SE:
            m_angle = 0.75f;
            MAX_ANGLE = 1.5f;
	        MIN_ANGLE = 0.0f;
        break;
        case SW: // Good
            m_angle = -0.75f;
            MAX_ANGLE = 0.0f;
	        MIN_ANGLE = -1.575f;
        break;
    }

    switch(m_type){
        case CAM_SPOT:
            m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
        break;
        case CAM_SWEEP:
            m_end = (Vector2){m_origin.x, m_origin.y + m_length};
        break;
    }
    
    m_direction = Vector2Normalize((Vector2){m_end.x, m_end.y});

	m_body = {t_x, t_y, WIDTH, HEIGHT}; // Temp

    m_isActive = true;
	m_playerDetected = false;
    m_movingRight = true;

    m_laser.p = c2V(m_origin.x, m_origin.y);// Starting point
    m_laser.d = c2V(m_direction.x , m_direction.y); // Direction
    m_laser.t = m_length; // Length

    angleV = 0.015f;
}

bool SecurityCamera::raycastPlayerCollision(Vector2& t_center){
    c2Circle player;
    c2Raycast laserCast;
    player.p = c2V(t_center.x, t_center.y);
    player.r = 30.0f;

    return c2RaytoCircle(m_laser, player, &laserCast);
}
void SecurityCamera::drawRaycast()
{
    float r = 2.0f;
	DrawLineEx((Vector2){m_laser.p.x, m_laser.p.y}, m_end, r, RED);
}

void SecurityCamera::update(float t_dt, Vector2 playerPos){
    if (!m_isActive) return;
    
    if(m_angle >= MAX_ANGLE && angleV > 0.0f){
        angleV *= -1;
        m_movingRight = false;
    }
    if(m_angle <= MIN_ANGLE && angleV <= 0.0f){
        angleV *= -1;
        m_movingRight = true;
    }
    m_angle += angleV; 
    
    switch(m_type){
        case CAM_SWEEP:
            if(m_movingRight == true){m_end.x += extendSpd;}
            else{m_end.x -= extendSpd;}
        break;
        case CAM_SPOT:
            m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
        break;
        default:
        break;
    }

    m_direction = Vector2Normalize((Vector2){m_end.x - m_origin.x, m_end.y - m_origin.y});
    m_laser.d = c2V(m_direction.x , m_direction.y);
    m_playerDetected = raycastPlayerCollision(playerPos);
}

void SecurityCamera::SetLevel(LevelData* level)
{
    m_level = level;
}

bool SecurityCamera::isPlayerDetected() const
{
    return m_playerDetected;
}

void SecurityCamera::draw()
{
	DrawRectangleRec(m_body, m_isActive ? ORANGE : DARKGRAY);

	if (!m_isActive) return;

    drawRaycast();

	if (m_playerDetected)
	{
        printf("Player Detected\n");
    	DrawCircleV(m_origin, 6, GREEN);
	}
}