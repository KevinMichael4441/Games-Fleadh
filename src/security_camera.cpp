#include "security_camera.h"

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd);

SecurityCamera::SecurityCamera()
{ 
}


void SecurityCamera::initialize(float t_x, float t_y, float t_maxRotation, float t_minRotation)
{
	m_body = {t_x, t_y, m_width, m_height};
	m_origin = { t_x + m_width / 2.0f, t_y + m_height / 2.0f };

    m_maxAngle = DEG2RAD * t_maxRotation;
	m_minAngle = DEG2RAD * t_minRotation;

	m_angle = m_minAngle;
	m_range = 250.0f;

	m_speed =  DEG2RAD * (rand() % 20 + 20); //20 to 40 degrees per second

    m_isActive = true;
	m_activeDuration = 5.0f;
    m_inactiveDuration = 5.0f;
	m_timer = 0.0f;

	m_playerDetected = false;
}


void SecurityCamera::update(float t_dt, Vector2 playerPos)
{
	m_timer += t_dt;

	if (m_isActive && m_timer >= m_activeDuration)
    {
        m_isActive = false;
        m_timer = 0.0f;
    }
    else if (!m_isActive && m_timer >= m_inactiveDuration)
    {
        m_isActive = true;
        m_timer = 0.0f;
    }

	if (m_isActive)
	{
    	m_angle += m_speed * t_dt;

    	if (m_angle >= m_maxAngle)
    	{
        	m_angle = m_maxAngle;
        	m_speed *= -1;
    	}
    	else if (m_angle <= m_minAngle)
    	{
        	m_angle = m_minAngle;
        	m_speed *= -1;
    	}
	}
	
	m_playerDetected = false;

    if (!m_isActive) return;

	Vector2 beamEnd = { m_origin.x + cosf(m_angle) * m_range, m_origin.y + sinf(m_angle) * m_range };

	const float playerRadius = 12.0f;

	if (Cam_CheckCollision_Player(playerPos, playerRadius, m_origin, beamEnd))
	{
    	m_playerDetected = true;
	}
}

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd)
{
	Vector2 line = Vector2Subtract(t_lineEnd, t_lineStart);
    Vector2 toCircle = Vector2Subtract(t_center, t_lineStart);

    float lineLengthSquared = line.x * line.x + line.y * line.y;

    if (lineLengthSquared <= 0.0001f) return false;

    float t = Vector2DotProduct(toCircle, line) / lineLengthSquared;

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    Vector2 closest = { t_lineStart.x + line.x * t, t_lineStart.y + line.y * t };

    float dx = t_center.x - closest.x;
    float dy = t_center.y - closest.y;

    float distanceSquared = dx * dx + dy * dy;

    return distanceSquared <= t_radius * t_radius;
}

bool SecurityCamera::isPlayerDetected() const
{
    return m_playerDetected;
}

void SecurityCamera::draw()
{
	DrawRectangleRec(m_body, m_isActive ? ORANGE : DARKGRAY);

	if (!m_isActive) return;

	Vector2 end = { m_origin.x + cosf(m_angle) * m_range, m_origin.y + sinf(m_angle) * m_range };
	DrawLineV(m_origin, end, RED);

	if (m_playerDetected)
	{
    	DrawCircleV(m_origin, 6, GREEN);
	}
}