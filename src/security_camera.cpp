#include "security_camera.h"

static bool Cam_CheckCollision_Player( Vector2 t_center, float t_radius, Vector2 t_lineStart, Vector2 t_lineEnd);

SecurityCamera::SecurityCamera()
{ 
}


void SecurityCamera::initialize(float t_x, float t_y, float t_maxRotation, float t_minRotation)
{
    m_maxAngle = 60;
	m_minAngle = 120;

	m_angle = m_minAngle;

	m_ray.direction.x = cos(m_angle);
	m_ray.direction.y = sin(m_angle);


	m_body = {t_x, t_y, m_width, m_height};
	m_origin = { t_x + m_width / 2.0f, t_y + m_height / 2.0f };

	m_ray.position = {m_origin.x, m_origin.y, 0};
	m_actualEndPoint.x = m_origin.x + (m_range * cos(DEG2RAD * m_angle));
	m_actualEndPoint.y = m_origin.y + (m_range * sin(DEG2RAD * m_angle));

	m_visualEndPoint = m_actualEndPoint;

	m_speed =  (rand() % 20 + 20);

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

	m_actualEndPoint.x = m_origin.x + (m_range * cos(m_angle));
	m_actualEndPoint.y = m_origin.y + (m_range * sin(m_angle));
	
	m_playerDetected = false;

    if (!m_isActive) return;

	FindBoundaryAABBs(m_origin.x, m_origin.y, m_actualEndPoint.x, m_actualEndPoint.y);

}

void SecurityCamera::SetLevel(LevelData* level)
{
    m_level = level;
}


void SecurityCamera::FindBoundaryAABBs(double x0, double y0, double x1, double y1) 
{
    if (!m_level)
	{
		return;
	}

	// Bresenham-based supercover line algorithm
	double dx = fabs(x1 - x0);
    double dy = fabs(y1 - y0);

    int x = int(floor(x0));
    int y = int(floor(y0));

    int n = 1;
    int x_inc, y_inc;
    double error;

    if (dx == 0)
    {
        x_inc = 0;
        error = std::numeric_limits<double>::infinity();
    }
    else if (x1 > x0)
    {
        x_inc = 32;
        n += (int(floor(x1)) - x) / 32;
        error = (floor(x0) + 32 - x0) * dy;
    }
    else
    {
        x_inc = -32;
        n += (x - int(floor(x1))) / 32;
        error = (x0 - floor(x0)) * dy;
    }

    if (dy == 0)
    {
        y_inc = 0;
        error -= std::numeric_limits<double>::infinity();
    }
    else if (y1 > y0)
    {
        y_inc = 32;
        n += (int(floor(y1)) - y) / 32;
        error -= (floor(y0) + 32 - y0) * dx;
    }
    else
    {
        y_inc = -32;
        n += (y - int(floor(y1))) / 32;
        error -= (y0 - floor(y0)) * dx;
    }

    for (; n > 0; --n)
    {
        if (Level_IsBoundaryPos(m_level, x, y))
		{
			RayCollision myCollision = GetRayCollisionBox(m_ray, {{(float)x,(float)y}, {(float)x+x_inc, (float)y+y_inc} });
			
			m_visualEndPoint.x = myCollision.point.x;
			m_visualEndPoint.y = myCollision.point.y;
			break;
		}

        if (error > 0)
        {
            y += y_inc;
            error -= dx;
        }
        else
        {
            x += x_inc;
            error += dy;
        }
    }
}


bool SecurityCamera::camCheckCollisionPlayer(Vector2 t_center)
{
	Vector2 line = Vector2Subtract(m_visualEndPoint, m_origin);
    Vector2 toCircle = Vector2Subtract(t_center, m_origin);

    float lineLengthSquared = line.x * line.x + line.y * line.y;

    if (lineLengthSquared <= 0.0001f) return false;

    float t = Vector2DotProduct(toCircle, line) / lineLengthSquared;

    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    Vector2 closest = { m_origin.x + line.x * t, m_origin.y + line.y * t };

    float dx = t_center.x - closest.x;
    float dy = t_center.y - closest.y;

    float distanceSquared = dx * dx + dy * dy;

    return distanceSquared <= BASE_RADIUS * BASE_RADIUS;
}

bool SecurityCamera::isPlayerDetected() const
{
    return m_playerDetected;
}

void SecurityCamera::draw()
{
	DrawRectangleRec(m_body, m_isActive ? ORANGE : DARKGRAY);

	if (!m_isActive) return;

	DrawLineV(m_origin, m_visualEndPoint, RED);

	if (m_playerDetected)
	{
    	DrawCircleV(m_origin, 6, GREEN);
	}
}


// void SecurityCamera::findIntersections(c2AABB t_tile) 
// {
// 	findIntersection({t_tile.min.x, t_tile.min.y}, {t_tile.max.x, t_tile.min.y});
// 	findIntersection({t_tile.max.x, t_tile.min.y}, {t_tile.max.x, t_tile.max.y});
// 	findIntersection({t_tile.max.x, t_tile.max.y}, {t_tile.min.x, t_tile.max.y});
// 	findIntersection({t_tile.min.x, t_tile.max.y}, {t_tile.min.x, t_tile.min.y});
// }

// void SecurityCamera::findIntersection(Vector2 t_start, Vector2 t_end) 
// {
// 	// SEGMENT in parametric: Point + Direction*T2
// 	float dx = t_end.x - t_start.x;
// 	float dy = t_end.y - t_start.y;

// 	// Are they parallel? If so, no intersect
// 	float r_mag = sqrt(m_ray.d.x * m_ray.d.x + m_ray.d.y * m_ray.d.y);
// 	float s_mag = sqrt(dx * dx + dy * dy);
	
// 	if(m_ray.d.x/r_mag == dx/s_mag && m_ray.d.y/r_mag == dy/s_mag)
// 	// Directions are the same.
// 	{ 
// 		return;
// 	}

// 	// SOLVE FOR T1 & T2
// 	// r_px+r_dx*T1 = s_px+s_dx*T2 && r_py+r_dy*T1 = s_py+s_dy*T2
// 	// ==> T1 = (s_px+s_dx*T2-r_px)/r_dx = (s_py+s_dy*T2-r_py)/r_dy
// 	// ==> s_px*r_dy + s_dx*T2*r_dy - r_px*r_dy = s_py*r_dx + s_dy*T2*r_dx - r_py*r_dx
// 	// ==> T2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx)
// 	float T2 = (m_ray.d.x*(t_start.y-m_ray.p.y) + m_ray.d.y*(m_ray.p.x-t_start.x))/(dx*m_ray.d.y - dy*m_ray.d.x);
// 	float T1 = (t_start.x+dx*T2-m_ray.p.x)/m_ray.d.x;

// 	// Must be within parametic whatevers for RAY/SEGMENT
// 	if(T1<0) return;
// 	if(T2<0 || T2>1) return;

// 	// Add to the POINTs OF INTERSECTION
// 	m_intersections.emplace_back((Intersection){{m_ray.p.x + m_ray.d.x * T1, m_ray.p.y + m_ray.d.y * T1}, T1});
// }

// void SecurityCamera::findClosestIntersection()
// {
// 	// Find CLOSEST intersection
// 	int numIntersections = m_intersections.size();
// 	Intersection closestIntersect = m_intersections[0];


// 	for(int index = 0; index < numIntersections; index++)
// 	{
// 		if(m_intersections[index].m_T1 < closestIntersect.m_T1)
// 		{
// 			closestIntersect = m_intersections[index];
// 		}
// 	}

// 	m_endPoint = closestIntersect.m_pointOfIntersection;
// }