#include "ooze.hpp"

Ooze::Ooze() 
{
}

void Ooze::Initialize(float t_k, float t_damp, Vector2 t_center, float t_speed, float t_jumpAmount)
{
	m_springConstant = t_k;
	m_damp = t_damp;
	m_centrePoint = t_center;
	m_speed = t_speed;
	m_jumpAmount = t_jumpAmount;

	m_gravity = {0.0f, 0.1f};

	m_currentState = STATE_IDLE;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		Point point = {
			{0,0},
			{0,0},
			{(float)(rand() % 30 + SCREEN_WIDTH/2 - 15), (float)(rand() % 30 + SCREEN_HEIGHT/2 - 15)},

			((float)(rand() % 20 + 20)),
			((float)(rand() % 20 + 20)),
			0.0f,

			0.0f,
			7.5f
		};

		point.m_mass = point.m_radius * point.m_radius;

		m_points[index] = point;
	}


	for (int currentPoint = 0; currentPoint < MAX_POINTS; currentPoint++)
	{
		for (int nextPoint = currentPoint + 1 ; nextPoint < MAX_POINTS; nextPoint++)
		{
			Point *a = &m_points[currentPoint];
			Point *b = &m_points[nextPoint];

			float xDistance = {b->m_position.x - a->m_position.x};
			float yDistance = {b->m_position.y - a->m_position.y};
			float magnitudeOfDistance = sqrt((xDistance * xDistance) + (yDistance * yDistance));
			Spring spring =  { m_springConstant, magnitudeOfDistance, a, b} ;


			switch (currentPoint)
			{
				case 0:
					m_springs[nextPoint - 1] = spring;
					break;
				case 1:
					m_springs[nextPoint + 4] = spring;
					break;
				case 2:
					m_springs[nextPoint + 8] = spring;
					break;
				case 3:
					m_springs[nextPoint + 11] = spring;
					break;
				case 4:
					m_springs[nextPoint + 13] = spring;
					break;
				case 5:
					m_springs[nextPoint + 14] = spring;
					break;			
			}
		}	
   	}
}

void Ooze::Update(float t_dt, Command t_activeCommand)
{
	HandleInput(t_activeCommand);
	UpdateState(t_dt);
}

void Ooze::UpdateState(float t_dt)
{
	if (m_currentState == STATE_IDLE)
	{
		UpdateIdleState(t_dt);
	}
	else if (m_currentState == STATE_MOVING)
	{
		UpdateMovingState(t_dt);
	}
	else if (m_currentState == STATE_JUMPING)
	{
		UpdateJumpingState(t_dt);
	}
}

void Ooze::UpdateIdleState(float t_dt)
{
	DefaultUpdate(t_dt);
}

void Ooze::UpdateMovingState(float t_dt)
{
	Move();
	DefaultUpdate(t_dt);
}

void Ooze::UpdateJumpingState(float t_dt)
{
	DefaultUpdate(t_dt);
	for (int index = 0; index < MAX_POINTS; index++)
 	{
		Jump();
	}

	HandleEvent(EVENT_INSTANT); 
}


void Ooze::DefaultUpdate(float t_dt)
{
	UpdateSprings();
	UpdatePoints(t_dt);
	ResolveBoundaryCollision_C2();
}

void Ooze::HandleInput(Command t_activeCommand)
{
	float axis = 0.0f;

	if (IsCommandActive(t_activeCommand, MOVE_LEFT))
		axis += -1.0f;

	if (IsCommandActive(t_activeCommand, MOVE_RIGHT))
		axis += 1.0f;


	// Check if we are moving
	bool moving = abs(axis) > 0.0f;
	bool jumping = IsCommandActive(t_activeCommand, ACTION_JUMP);

	// EVENT_MOVE only of not EVENT_ATTACK and NOT EVENT_DEFEND
	if (!jumping && moving)
	{
		m_moveDirection = axis;

		HandleEvent(EVENT_MOVE);
	}
	// Send none if not MOVING or Performing and ACTION
	else if (!moving && !jumping)
	{
		HandleEvent(EVENT_NONE);
	}

	// ATTACK / DEFEND Send last
	if (jumping)
	{
		HandleEvent(EVENT_JUMP);
	}
}	

void Ooze::HandleEvent(Event t_event)
{
	if (fsm.CheckValidTransition(m_currentState, t_event))
	{
		ExitState();
		m_currentState = fsm.getState();
		EnterState(m_currentState);
	}
}

void Ooze::ExitState()
{
	switch (m_currentState)
	{
		case STATE_IDLE:
			ExitIdleState();
			break;
		case STATE_MOVING:
			ExitMoveState();
			break;
		case STATE_JUMPING:
			ExitJumpState();
			break;
	}
}

void Ooze::ExitIdleState()
{
	std::cout << "Exited Idle State\n";
}

void Ooze::ExitJumpState()
{
	std::cout << "Exited Jump State\n";
}

void Ooze::ExitMoveState()
{
	std::cout << "Exited Move State\n";
}

void Ooze::EnterState(State t_stateToEnter)
{
	switch (t_stateToEnter)
	{
		case STATE_IDLE:
			EnterIdleState();
			break;
		case STATE_MOVING:
			EnterMoveState();
			break;
		case STATE_JUMPING:
			EnterJumpState();
			break;
	}
}

void Ooze::EnterIdleState()
{
	std::cout << "Entered Idle State\n";
}

void Ooze::EnterMoveState()
{
	std::cout << "Entered Move State\n";
}

void Ooze::EnterJumpState()
{
	std::cout << "Entered Jump State\n";
}

void Ooze::UpdateSprings()
{
	for (int index = 0; index < MAX_SPRINGS; index++)
	{

		float xForce = m_springs[index].b->m_position.x - m_springs[index].a->m_position.x;
		float yForce = m_springs[index].b->m_position.y - m_springs[index].a->m_position.y;
		float magnitudeOfForce = sqrt((xForce * xForce) + (yForce * yForce));
		float displacement = magnitudeOfForce - m_springs[index].restLength;

		Vector2 normalizedForce;
		if (magnitudeOfForce == 0)
		{
			normalizedForce.x = 0;
			normalizedForce.y = 0;
		}
		else
		{
			normalizedForce.x = xForce / magnitudeOfForce;
			normalizedForce.y = yForce / magnitudeOfForce;
		}
		normalizedForce.x *= m_springs[index].springConstant * displacement;
		normalizedForce.y *= m_springs[index].springConstant * displacement;

		float sumOfMass = m_springs[index].a->m_mass + m_springs[index].b->m_mass;
		float weightOfA = m_springs[index].b->m_mass / sumOfMass;
		float weightOfB = m_springs[index].a->m_mass / sumOfMass;

		m_springs[index].a->m_acceleration.x += normalizedForce.x * weightOfA;
		m_springs[index].a->m_acceleration.y += normalizedForce.y * weightOfA;
		
		normalizedForce.x *= -1;
		normalizedForce.y *= -1;

		m_springs[index].b->m_acceleration.x += normalizedForce.x * weightOfB;
		m_springs[index].b->m_acceleration.y += normalizedForce.y * weightOfB;
	}
}

void Ooze::UpdatePoints(float t_dt)
{
	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].m_acceleration.y += (m_gravity.y * m_points[index].m_radius / 10);

		m_points[index].m_velocity.x *= m_damp;
		m_points[index].m_velocity.y *= m_damp;
	
		m_points[index].m_velocity.x += m_points[index].m_acceleration.x;
		m_points[index].m_velocity.y += m_points[index].m_acceleration.y;

		m_points[index].m_position.x += m_points[index].m_velocity.x;
		m_points[index].m_position.y += m_points[index].m_velocity.y;

		//ClampPlayerOnScreen(index); //deltaTime, index);

		m_points[index].m_acceleration.x = 0;
		m_points[index].m_acceleration.y = 0;	



		m_points[index].lerpTimeElapsed += t_dt;
		m_points[index].m_radius = Lerp(m_points[index].m_radius, m_points[index].m_newRadius,
			m_points[index].lerpTimeElapsed / m_points[index].lerpTime);

		float difference = m_points[index].m_newRadius - m_points[index].m_radius;
		if (difference < 0.01 && difference > -0.01)
		{
			m_points[index].m_newRadius = rand() % 20 + 20;
			m_points[index].lerpTimeElapsed = 0;
		}
	}
}


/*void Ooze::ClampPlayerOnScreen(int index)
{
	if (m_points[index].m_position.x < m_points[index].m_radius)
	{
		m_points[index].m_position.x = m_points[index].m_radius;
	}	
			
	if (m_points[index].m_position.x > SCREEN_WIDTH - m_points[index].m_radius)
	{
		m_points[index].m_position.x = SCREEN_WIDTH - m_points[index].m_radius;
	}
			
	if (m_points[index].m_position.y < m_points[index].m_radius)
	{
		m_points[index].m_position.y = m_points[index].m_radius;
	}
			
	if (m_points[index].m_position.y > SCREEN_HEIGHT - m_points[index].m_radius)
	{
		m_points[index].m_position.y = SCREEN_HEIGHT - m_points[index].m_radius;
	}
}*/

void Ooze::Jump()
{
	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].m_velocity.y -= (m_jumpAmount * m_points[index].m_radius / 10);;
	}
}

void Ooze::Move()
{
	for (int index = 0; index < MAX_POINTS; index++)
 	{
		m_points[index].m_velocity.x += (m_moveDirection * m_speed * m_points[index].m_radius / 10);
	}
}

void Ooze::Spread()
{
	for (int index = 0; index < MAX_POINTS / 2; index++)
	{
		m_points[index].m_velocity.x -= (m_jumpAmount * m_points[index].m_radius / 10);;
	}
	
	for (int index = MAX_POINTS / 2; index < MAX_POINTS; index++)
	{
		m_points[index].m_velocity.x += (m_jumpAmount * m_points[index].m_radius / 10);;
	}
}

Vector2 Ooze::CalculateCenter()
{
	float sumX = 0.0f;
	float sumY = 0.0f;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		sumX += m_points[index].m_position.x;
		sumY += m_points[index].m_position.y;
	}

	float x = sumX / MAX_POINTS;
	float y = sumY / MAX_POINTS;
	
	return (Vector2){x,y};
}

void Ooze::Draw()
{
	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radius, (Color){ 0, 210, 0, 255 });
	}

	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radius / 1.5, (Color){ 0, 190, 0, 255 });
	}

	
	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radius / 3, (Color){ 0, 180, 0, 255 });
	}
}

void Ooze::SetLevel(LevelData* level)
{
    m_level = level;
}

// used to find boundary tiles in a 3x3 grid around oozes circles
int Ooze::FindBoundaryAABBs(Vector2 centrePos, float radius, c2AABB liveAABBs[MAX_BOUNDARY_RECTS]) const
{
    if (!m_level)
	{
		return 0;
	}

    int tileX = (int)(centrePos.x / m_level->tileWidth);
    int tileY = (int)(centrePos.y / m_level->tileHeight);

    int count = 0;

    for (int oy = -1; oy <= 1; oy++)
    {
        for (int ox = -1; ox <= 1; ox++)
        {
			// adjust our position (in array)
            int checkTileX = tileX + ox;
            int checkTileY = tileY + oy;

			// convert that array position to a realworld position in the tiles centre
            float tileCentreX = (checkTileX * m_level->tileWidth) + (m_level->tileWidth  * 0.5f);
            float tileCentreY = (checkTileY * m_level->tileHeight) + (m_level->tileHeight * 0.5f);

            if (Level_IsBoundaryPos(m_level, tileCentreX, tileCentreY)) // if it's a boundary tile
            {
                if (count < MAX_BOUNDARY_RECTS)
                {
					// get coords of the tiles edges
                    float left = (float)(checkTileX * m_level->tileWidth);
                    float top = (float)(checkTileY * m_level->tileHeight);
                    float right = left + (float)m_level->tileWidth;
                    float bottom = top + (float)m_level->tileHeight;

					// make rectangle using those coords
                    c2AABB a;
                    a.min = c2V(left, top);
                    a.max = c2V(right, bottom);

					// store it
                    liveAABBs[count] = a;
                    count++;
                }
            }
        }
    }

    return count;
}

static void ResolvePointVsAABB(Point& p, const c2AABB& rec, float slop, float str, float friction)
{
    c2Circle circle = { c2V(p.m_position.x, p.m_position.y), p.m_radius };	// convert Ooze point into c2Circle 

    c2Manifold m = {};	// create manifold object

	// is the circle and rec overlapping
    c2CircletoAABBManifold(circle, rec, &m);
    
	// count is contact points
	if (m.count == 0)
	{
		return;
	}

	// get the penetration depth of the overlap
    float depth = m.depths[0];
    if (depth <= slop)
	{
		return;
	}

    // push point out of rec
    float push = (depth - slop) * str;
    p.m_position.x -= m.n.x * push;
    p.m_position.y -= m.n.y * push;
}

void Ooze::ResolveBoundaryCollision_C2()
{
    if (!m_level)
	{
		return;
	}

    const int iterations = 6;		// repeating a collision resolution multiple times per frame is apparently a good idea for soft bodies so...
    const float slop = 0.75f;		// an allowance of overlap. Helps smooth things out
    const float str = 0.75f;		// push strength
    const float friction = 0.20f;	// reduce sliding  

    for (int iter = 0; iter < iterations; iter++)	// for each iteration
    {
        for (int p = 0; p < MAX_POINTS; p++)		// for each point
        {
            Point& point = m_points[p];

			// make list of surrounding AABBs
            c2AABB tiles[MAX_BOUNDARY_RECTS];
            int count = FindBoundaryAABBs(point.m_position, point.m_radius, tiles);

			// resolve each point against AABB
            for (int t = 0; t < count; t++)
			{
                ResolvePointVsAABB(point, tiles[t], slop, str, friction);
			}
        }
    }
}