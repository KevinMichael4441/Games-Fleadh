#include "security_camera.h"

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd);

SecurityCamera::SecurityCamera()
{ 
}


void SecurityCamera::initialize(float t_x, float t_y)
{
    m_origin = { t_x + WIDTH / 2.0f, t_y + HEIGHT / 2.0f };
	m_angle = 0.0f;
    m_length = MIN_LENGTH;
    m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
    m_direction = Vector2Normalize((Vector2){m_end.x, m_end.y});

	m_body = {t_x, t_y, WIDTH, HEIGHT};

    m_isActive = true;
	m_playerDetected = false;
    m_movingRight = true;

    m_laser.p = c2V(m_origin.x, m_origin.y);// Starting point
    m_laser.d = c2V(m_direction.x , m_direction.y); // Direction
    m_laser.t = m_length; // Length

    angleV = 0.02f;
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

void SecurityCamera::update(float t_dt, Vector2 playerPos)
{
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

    if(m_angle > 0.0f)
    {
        if(m_movingRight == true)
        {
            if(m_length < MAX_LENGTH)
            {
                m_length += extendSpd;
            }
        }
        else
        {
            if(m_length > MIN_LENGTH)
            {
                m_length -= extendSpd;
            }
        }
    }
    else
    {
        if(m_movingRight == false)
        {
            if(m_length < MAX_LENGTH)
            {
                m_length += extendSpd;
            }
        }
        else
        {
            if(m_length > MIN_LENGTH)
            {
                m_length -= extendSpd;
            }
        }
    }


    m_end = (Vector2){sin(m_angle) * m_length + m_origin.x, cos(m_angle) * m_length + m_origin.y};
    m_direction = Vector2Normalize((Vector2){m_end.x - m_origin.x, m_end.y - m_origin.y});
    m_laser.d = c2V(m_direction.x , m_direction.y); // Direction

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