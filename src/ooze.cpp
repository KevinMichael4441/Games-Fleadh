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
	m_squishiness = SQUISH_AMOUNT::NOP;

	for (int index = 0; index < MAX_COLLISION_PARTS; index++)
	{
		collisionParts[index] = false;
	}

	m_collisionTimer = 0;
	m_toCollideTimer = 0.0f;

	m_gravity = {0.0f, GRAVITY};

	for (int index = 0; index < MAX_POINTS; index++)
	{
		Point point = {
			{0,0},
			{0,0},
			{(float)((float)(rand() % 30 + m_centrePoint.x - 15)), (float)(rand() % 30 + m_centrePoint.y - 15)},

			((float)(rand() % RAND_RADIUS + BASE_RADIUS)),
			((float)(rand() % RAND_RADIUS + BASE_RADIUS)),

			((float)(rand() % RAND_RADIUS + BASE_RADIUS)),
			((float)(rand() % RAND_RADIUS + BASE_RADIUS)),
			0.0f,

			0.0f,
			0.0f,

			7.5f,
		};

		point.m_mass = point.m_radiusX * point.m_radiusX;
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

void Ooze::HandleInput(Command t_activeCommand)
{
	float axis = 0.0f;

	if (IsCommandActive(t_activeCommand, MOVE_LEFT))
		axis += -1.0f;

	if (IsCommandActive(t_activeCommand, MOVE_RIGHT))
		axis += 1.0f;

	if (IsCommandActive(t_activeCommand, AIM_LEFT))
		axis += -4.0f;

	if (IsCommandActive(t_activeCommand, AIM_RIGHT))
		axis += 4.0f;

	// Check if we are moving
	bool moving = abs(axis) > 0.0f;
	bool jumping = IsCommandActive(t_activeCommand, ACTION_JUMP);

	m_moveDirection = axis;

	if (!jumping && moving)
	{
		HandleEvent(EVENT_MOVE);
	}
	else if (!moving && !jumping)
	{
		HandleEvent(EVENT_NONE);
	}

	if (jumping)
	{
		if (HandleEvent(EVENT_JUMP))
		{
			for (int index = 0; index < MAX_POINTS; index++)
 			{
				Jump();
			}
		}

	}
}	

bool Ooze::HandleEvent(Event t_event)
{
	fsm.m_previousState = fsm.m_currentState;
	if (fsm.CheckValidTransition(fsm.m_currentState, t_event))
	{
		ExitState();
		EnterState(fsm.m_currentState, t_event);
		return true;
	}

	return false;
}

void Ooze::Update(float t_dt, Command t_activeCommand)
{
	HandleInput(t_activeCommand);
	UpdateState(t_dt);
}

void Ooze::UpdateState(float t_dt)
{
	switch (fsm.m_currentState)
	{
		case STATE_IDLE:
			UpdateIdleState(t_dt);
			break;
		case STATE_MOVING:
			UpdateMoveState(t_dt);
			break;
		case STATE_JUMPING:
			UpdateJumpState(t_dt);
			break;
		case STATE_COLLIDE_HORIZONTAL:
			UpdateCollideHorizontalState(t_dt);
			break;
		case STATE_COLLIDE_DOWN:
		case STATE_COLLIDE_UP:
			UpdateCollideVerticalState(t_dt);
			break;
	}
}

void Ooze::UpdateIdleState(float t_dt)
{
	DefaultUpdate(t_dt);
}

void Ooze::UpdateMoveState(float t_dt)
{
	DefaultUpdate(t_dt);
	m_toCollideTimer += t_dt;
}

void Ooze::UpdateJumpState(float t_dt)
{
	DefaultUpdate(t_dt);
	m_toCollideTimer += t_dt;
}

void Ooze::UpdateCollideHorizontalState(float t_dt)
{
	m_collisionTimer += t_dt;
	
	switch (m_squishiness)
	{
		case SQUISH_AMOUNT::LOW:
			LowHorizontalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::MEDIUM:
			MediumHorizontalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::HIGH:
			HighHorizontalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::NOP:
			break;
	}

	DefaultUpdate(t_dt);
}

void Ooze::LowHorizontalCollisionAnimation()
{
	if (m_collisionTimer > 0.6f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.x = 0;
		}
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		//X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 6 + 18;
			m_points[index].m_newRadiusX = rand() % 6 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 12 + 36;
			m_points[index].m_newRadiusX = rand() % 3 + 9;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::MediumHorizontalCollisionAnimation()
{
	if (m_collisionTimer > 0.6f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.x = 0;
		}
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		//X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 6 + 24;
			m_points[index].m_newRadiusX = rand() % 3 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 12 + 48;
			m_points[index].m_newRadiusX = rand() % 3 + 6;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::HighHorizontalCollisionAnimation()
{
	if (m_collisionTimer > 0.8f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.x = 0;
		}
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.6f && !collisionParts[3])
	{
		//X and Y reset back
		//Circles pushed down
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[3] = true;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		//X is slightly more than normal
		//Y is higher
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY= rand() % 3 + 15;
			m_points[index].m_newRadiusX = rand() % 6 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		//X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 6 + 30;
			m_points[index].m_newRadiusX = rand() % 6 + 12;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusY = rand() % 12 + 60;
			m_points[index].m_newRadiusX = 3;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::LowVerticalCollisionAnimation()
{
	if (m_collisionTimer > 0.6f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.y = 0;
		}

		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 6 + 18;
			m_points[index].m_newRadiusY = rand() % 6 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 12 + 36;
			m_points[index].m_newRadiusY = rand() % 3 + 9;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::MediumVerticalCollisionAnimation()
{
	if (m_collisionTimer > 0.6f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.y = 0;
		}
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 6 + 24;
			m_points[index].m_newRadiusY = rand() % 3 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 12 + 48;
			m_points[index].m_newRadiusY = rand() % 3 + 6;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::HighVerticalCollisionAnimation()
{
	if (m_collisionTimer > 0.8f)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.y = 0;
		}
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (m_collisionTimer > 0.6f && !collisionParts[3])
	{
		//X and Y reset back
		//Circles pushed down
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].m_newRadiusY = rand() % RAND_RADIUS + BASE_RADIUS;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[3] = true;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		//X is slightly more than normal
		//Y is higher
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 3 + 15;
			m_points[index].m_newRadiusY = rand() % 6 + 15;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		//X is reducing and Y is increasing
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 6 + 30;
			m_points[index].m_newRadiusY = rand() % 6 + 12;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 12 + 60;
			m_points[index].m_newRadiusY = 3;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}


void Ooze::UpdateCollideVerticalState(float t_dt)
{
	m_collisionTimer += t_dt;
	
	switch (m_squishiness)
	{
		case SQUISH_AMOUNT::LOW:
			LowVerticalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::MEDIUM:
			MediumVerticalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::HIGH:
			HighVerticalCollisionAnimation();
			break;
		case SQUISH_AMOUNT::NOP:
			break;
	}

	DefaultUpdate(t_dt);
}

void Ooze::DefaultUpdate(float t_dt)
{
	UpdateSprings();
	UpdatePoints(t_dt);
	if (fsm.m_currentState != STATE_COLLIDE_HORIZONTAL)
	{
		Move();
	}

	
	if (m_toCollideTimer < m_toCollideDelay)
	{
		if (fsm.m_currentState != STATE_JUMPING
			&& fsm.m_currentState != STATE_MOVING)
		{
			float avgVel = 0;
			for (int index = 0; index < MAX_POINTS; index++)
			{
				avgVel += m_points[index].m_velocity.y;
			}

			if (avgVel > 3)
			{
				HandleEvent(EVENT_JUMP);
			}
	}
	}
}

void Ooze::ExitState()
{
	switch (fsm.m_previousState)
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
		case STATE_COLLIDE_DOWN:
		case STATE_COLLIDE_UP:
		case STATE_COLLIDE_HORIZONTAL:
			ExitCollideState();
			break;
	}
}

void Ooze::ExitIdleState()
{
	std::cout << "Exited Idle State\n";
}

void Ooze::ExitJumpState()
{
	m_toCollideTimer = 0.0f;
	std::cout << "Exited Jump State\n";
}

void Ooze::ExitMoveState()
{
	m_toCollideTimer = 0.0f;
	std::cout << "Exited Move State\n";
}

void Ooze::ExitCollideState()
{
	std::cout << "Exited Collide State\n";

	//reset Collision Timer
	m_collisionTimer = 0.0f;

	//Reset lerpTime and lerpElapsedTime
	//Also reset extremePos flag
	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].lerpTime = 7.5f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

	//RESET REST LENGTH

	//reset collision part flag
	for (int index = 0; index < MAX_COLLISION_PARTS; index++)
	{
		collisionParts[index] = false;
	}
}



void Ooze::EnterState(State t_stateToEnter, Event t_event)
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
		case STATE_COLLIDE_HORIZONTAL:
			EnterCollideHorizontalState();
			break;
		case STATE_COLLIDE_DOWN:
		case STATE_COLLIDE_UP:
			EnterCollideVerticalState();
			break;
	}
}

void Ooze::EnterIdleState()
{
	std::cout << "Entered Idle State\n";
}

void Ooze::EnterMoveState()
{
	m_toCollideTimer = 0.0f;
	std::cout << "Entered Move State\n";
}

void Ooze::EnterJumpState()
{
	m_toCollideTimer = 0.0f;
	std::cout << "Entered Jump State\n";
}

void Ooze::EnterCollideHorizontalState()
{
	std::cout << "Entered Collide Left or Right State\n";

	float avgVelocity = 0.0f;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		avgVelocity += abs(m_points[index].m_velocity.x);

		m_points[index].lerpTime = 0.4f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

	avgVelocity /= MAX_POINTS;


	m_collisionTimer = 0.0f;
	if (avgVelocity < 6)
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			SetNewLerp(index, RAND_RADIUS, BASE_RADIUS, RAND_RADIUS, BASE_RADIUS);
		}
			
		HandleEvent(EVENT_TIMER);		
		return;
	}
	else if (avgVelocity < 12)
	{
		m_squishiness = SQUISH_AMOUNT::LOW;
	}
	else 
	{
		m_squishiness = SQUISH_AMOUNT::HIGH;
	}
}

void Ooze::EnterCollideVerticalState()
{
	std::cout << "Entered Collide Up or Down State\n";
	m_collisionTimer = 0.0f;

	float avgVelocity = 0.0f;

	for (int index = 0; index < MAX_POINTS; index++)
	{

		avgVelocity += abs(m_points[index].m_velocity.y);

		m_points[index].lerpTime = 0.4f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

	avgVelocity /= MAX_POINTS;

	if (avgVelocity < 6)
	{
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (avgVelocity < 12)
	{
		m_squishiness = SQUISH_AMOUNT::LOW;
	}
	else if (avgVelocity < 18)
	{
		m_squishiness = SQUISH_AMOUNT::MEDIUM;
	}
	else 
	{
		m_squishiness = SQUISH_AMOUNT::HIGH;
	}
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
		m_points[index].m_acceleration.y += (m_gravity.y * m_points[index].m_radiusX / 10);

		m_points[index].m_velocity.x *= m_damp;
		m_points[index].m_velocity.y *= m_damp;
	
		m_points[index].m_velocity.x += m_points[index].m_acceleration.x;
		m_points[index].m_velocity.y += m_points[index].m_acceleration.y;

		for (int iterations = ITERATIONS; iterations > 0; iterations--)
		{
			m_points[index].m_position.x += (m_points[index].m_velocity.x / ITERATIONS);
			m_points[index].m_position.y += (m_points[index].m_velocity.y / ITERATIONS);

			c2AABB tiles[MAX_BOUNDARY_RECTS];
            int count = FindBoundaryAABBs(m_points[index].m_position, tiles);

			// resolve each point against AABB
            for (int t = 0; t < count; t++)
			{
                ResolvePointVsAABB(m_points[index], tiles[t], 0.0f, 0.2f, 0.2f);
			}
		}

		m_points[index].m_acceleration.x = 0;
		m_points[index].m_acceleration.y = 0;	

		m_points[index].lerpTimeElapsedX += t_dt;
		m_points[index].lerpTimeElapsedY += t_dt;

		m_points[index].m_radiusX = Lerp(m_points[index].m_radiusX, m_points[index].m_newRadiusX,
			m_points[index].lerpTimeElapsedX / m_points[index].lerpTime);

		m_points[index].m_radiusY = Lerp(m_points[index].m_radiusY, m_points[index].m_newRadiusY,
			m_points[index].lerpTimeElapsedY / m_points[index].lerpTime);

		if (fsm.m_currentState == STATE_IDLE || 
			fsm.m_currentState == STATE_JUMPING ||
			fsm.m_currentState == STATE_MOVING)
		{
			SetNewLerp(index, RAND_RADIUS,BASE_RADIUS,RAND_RADIUS,BASE_RADIUS);	
		}
		
	}
}

void Ooze::SetNewLerp(int index, int t_randX, int t_baseX, int t_randY, int t_baseY)
{
	float differenceX = m_points[index].m_newRadiusX - m_points[index].m_radiusX;
	float differenceY = m_points[index].m_newRadiusY - m_points[index].m_radiusY;

	if (differenceX < 0.01 && differenceX > -0.01)
	{
		m_points[index].m_newRadiusX = rand() % t_randX + t_baseX;
		m_points[index].lerpTimeElapsedX = 0;
	}

	if (differenceY < 0.01 && differenceY > -0.01)
	{
		m_points[index].m_newRadiusY = rand() % t_randY + t_baseY;
		m_points[index].lerpTimeElapsedY = 0;
	}
}

void Ooze::Jump()
{
	float avgRadii = 0.0f;
	for (int index = 0; index < MAX_POINTS; index++)
	{
		avgRadii = (m_points[index].m_radiusX + m_points[index].m_radiusY) / 2;
		m_points[index].m_velocity.y -= (m_jumpAmount * avgRadii / 10);
	}
}

void Ooze::Move()
{
	float avgRadii;
	for (int index = 0; index < MAX_POINTS; index++)
 	{
		avgRadii = (m_points[index].m_radiusX + m_points[index].m_radiusY) / 2;
		m_points[index].m_velocity.x += (m_moveDirection * m_speed * avgRadii / 10);
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
		DrawEllipse(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radiusX, m_points[index].m_radiusY, (Color){ 0, 210, 0, 255 });
	}

	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawEllipse(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radiusX / 1.5,  m_points[index].m_radiusY / 1.5, (Color){ 0, 190, 0, 255 });
	}
	
	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawEllipse(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radiusX / 3,  m_points[index].m_radiusY / 3, (Color){ 0, 180, 0, 255 });
	}
}

void Ooze::SetLevel(LevelData* level)
{
    m_level = level;
}

// used to find boundary tiles in a 3x3 grid around oozes circles
int Ooze::FindBoundaryAABBs(Vector2 centrePos, c2AABB liveAABBs[MAX_BOUNDARY_RECTS]) const
{
    if (!m_level)
	{
		return 0;
	}

	// convert circle position into tile coordinates
    int tileX = (int)(centrePos.x / m_level->tileWidth);
    int tileY = (int)(centrePos.y / m_level->tileHeight);

    int count = 0;	// num of AABBs

    for (int oy = -1; oy <= 1; oy++)	// oy (-1, 0, 1) are the rows above, level with and below the circle 
    {
		const int y = tileY + oy;	// the current row we're checking

        bool midCombo = false;		// used to track if we're in a continuous "combo" (or chain?) of collidable tiles
        int comboSourceX = 0; 		// where the combo started
        int comboLength = 0;		// how many tiles are in the current combo

        for (int ox = -1; ox <= 1; ox++)	// ox (-1, 0, 1) are the columns left, level with and right of the circle
        {
            const int x = tileX + ox;	// the current col we're checking

            bool boundary = false;
            if (x >= 0 && y >= 0 && x < m_level->levelWidth && y < m_level->levelHeight)	// prevents checking outside of the array
            {
				// convert that array position to a realworld position in the tiles centre
                float tileCentreX = (x * m_level->tileWidth) + (m_level->tileWidth * 0.5f);
                float tileCentreY = (y * m_level->tileHeight) + (m_level->tileHeight * 0.5f);

                boundary = Level_IsBoundaryPos(m_level, tileCentreX, tileCentreY);	// is it a boundary tile
            }

            if (boundary == true)	// if it's a boundary tile
            {
                if (midCombo == false) // if we're not currently in a combo, start one
                {
                    midCombo = true;
                    comboSourceX = x;
                    comboLength = 1;
                }
                else	// otherwise extend the combo
                {
                    comboLength++;
                }
            }

            const bool endOfRow = (ox == 1);	// if this is the last column

            if (midCombo && (boundary == false || endOfRow == true)) // if we're mid combo and the combo ends or we reach max combo
            {
                if (count < MAX_BOUNDARY_RECTS)
                {
					// get coords of the tiles edges
                    float left   = (float)(comboSourceX * m_level->tileWidth);
                    float top    = (float)(y * m_level->tileHeight);
                    float right  = left + (float)(comboLength * m_level->tileWidth);
                    float bottom = top + (float)m_level->tileHeight;

					// make rectangle using those coords
                    c2AABB a;
                    a.min = c2V(left, top);
                    a.max = c2V(right, bottom);

					// store it
                    liveAABBs[count++] = a;
                }

				// reset
                midCombo = false;
                comboLength = 0;
            }
        }
    }

    return count;
}

void Ooze::ResolvePointVsAABB(Point& p, const c2AABB& rec, float slop, float str, float friction)
{
    c2Circle circleX = { c2V(p.m_position.x, p.m_position.y), p.m_radiusX };	// convert Ooze point into c2Circle 
	c2Circle circleY = { c2V(p.m_position.x, p.m_position.y), p.m_radiusY };	// convert Ooze point into c2Circle 

    c2Manifold mX = {};	// create manifold object
	c2Manifold mY = {};	// create manifold object

	// is the circle and rec overlapping
    c2CircletoAABBManifold(circleX, rec, &mX);
    c2CircletoAABBManifold(circleY, rec, &mY);
    
	// count is contact points
	if (mX.count == 0 && mY.count == 0)
	{
		return;
	}
	
	// get the penetration depth of the overlap
    float depthX = mX.depths[0];
	float depthY = mY.depths[0];

	//float depth = depthX > depthY ? depthX : depthY;
	//c2Manifold m = (depthX > depthY) ? mX : mY;	

    if (depthX <= slop && depthY <= slop)
	{
		return;
	}

	// push point out of rec
    float pushX = (depthX - slop) * str;
	float pushY = (depthY - slop) * str;

	if (mY.n.x > 0.1)
	{
		p.m_position.x -= pushX;
		if (fsm.m_currentState != STATE_COLLIDE_HORIZONTAL)
		{
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
		}
		p.m_velocity.x = 0;
	}
	else if(mY.n.x < -0.1)
	{
		p.m_position.x += pushX;
		if (fsm.m_currentState != STATE_COLLIDE_HORIZONTAL)
		{
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
		}
		p.m_velocity.x = 0;
	}
		
	else if (mY.n.y > 0.1)
	{
		p.m_position.y -= pushY;
		if (fsm.m_currentState != STATE_COLLIDE_DOWN)
		{
			HandleEvent(EVENT_COLLIDE_DOWN);
		}	
		p.m_velocity.y = 0;
	}
	else if (mY.n.y < -0.1)
	{
		p.m_position.y += pushY;
		if (fsm.m_currentState != STATE_COLLIDE_UP)
		{
			HandleEvent(EVENT_COLLIDE_UP);
		}
		p.m_velocity.y = 0;
	}
}

void Ooze::Reset(Vector2 startPos)
{
    m_centrePoint = startPos;
	ExitState();
	EnterState(STATE_IDLE, EVENT_NONE);

    float angleStep = 2.0f * PI / MAX_POINTS;
    float radius = BASE_RADIUS;

    for (int i = 0; i < MAX_POINTS; i++)
    {
        float angle = i * angleStep;

        Vector2 offset =
        {
            cosf(angle) * radius,
            sinf(angle) * radius
        };

        m_points[i].m_position = Vector2Add(startPos, offset);

        m_points[i].m_velocity = (Vector2){0,0};
        m_points[i].m_acceleration = (Vector2){0,0};
    }
}

Vector2 Ooze::getPosition()
{
    return CalculateCenter();
}

Point* Ooze::GetPoints()
{
    return m_points;
}

int Ooze::GetPointCount() const
{
    return MAX_POINTS;
}


//----------------KNOWN ISSUES-------------------//
/*
1 - Jitter (Could be potentially solved by slightly adjusting camera logic)	(Isn't fully solved; need new ideas)
*/
//----------------KNOWN ISSUES-------------------//