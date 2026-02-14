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

	for (int index = 0; index < MAX_POINTS; index++)
	{
		Point point = {
			{0,0},
			{0,0},
			{(float)(rand() % 30 + SCREEN_WIDTH/2 - 15), (float)(rand() % 30 + SCREEN_HEIGHT/2 - 15)},

			((float)(rand() % 20 + 20)),
			((float)(rand() % 20 + 20)),

			((float)(rand() % 20 + 20)),
			((float)(rand() % 20 + 20)),
			0.0f,

			0.0f,
			0.0f,

			7.5f
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
			Spring spring =  { m_springConstant, magnitudeOfDistance, magnitudeOfDistance, a, b} ;


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
		HandleEvent(EVENT_JUMP);
	}
}	

void Ooze::HandleEvent(Event t_event)
{
	if (fsm.CheckValidTransition(fsm.m_currentState, t_event))
	{
		ExitState();
		EnterState(fsm.m_currentState);
	}
}

void Ooze::Update(float t_dt, Command t_activeCommand)
{
	HandleInput(t_activeCommand);


	UpdateState(t_dt);

	if (IsKeyPressed(KEY_X))
	{
		Spread();
	}
	
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
			UpdateCollideDownState(t_dt);
			break;
		case STATE_COLLIDE_UP:
			UpdateCollideUpState(t_dt);
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
	Move();
}

void Ooze::UpdateJumpState(float t_dt)
{
	DefaultUpdate(t_dt);
	Move();
}

void Ooze::UpdateCollideHorizontalState(float t_dt)
{
	m_collisionTimer += t_dt;

	if (m_collisionTimer > 0.1f)
	{
		HandleEvent(EVENT_TIMER);
	}
}

void Ooze::UpdateCollideUpState(float t_dt)
{
	m_collisionTimer += t_dt;

	if (m_collisionTimer > 0.1f)
	{
		HandleEvent(EVENT_TIMER);
	}
}

void Ooze::UpdateCollideDownState(float t_dt)
{
	m_collisionTimer += t_dt;

	if (m_collisionTimer > 0.1f)
	{
		HandleEvent(EVENT_TIMER);
	}
}

void Ooze::DefaultUpdate(float t_dt)
{
	UpdateSprings();
	UpdatePoints(t_dt);
}


void Ooze::ExitState()
{
	switch (fsm.m_currentState)
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
		case STATE_COLLIDE_HORIZONTAL:
			ExitCollideHorizontalState();
			break;
		case STATE_COLLIDE_DOWN:
			ExitCollideDownState();
			break;
		case STATE_COLLIDE_UP:
			ExitCollideUpState();
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

void Ooze::ExitCollideHorizontalState()
{
	std::cout << "Exited CollideHorizontal State\n";
	m_collisionTimer = 0.0f;
}

void Ooze::ExitCollideUpState()
{
	std::cout << "Exited Collide Up State\n";
	m_collisionTimer = 0.0f;
}

void Ooze::ExitCollideDownState()
{
	std::cout << "Exited Collide Down State\n";
	m_collisionTimer = 0.0f;
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
		case STATE_COLLIDE_HORIZONTAL:
			EnterCollideHorizontalState();
			break;
		case STATE_COLLIDE_DOWN:
			EnterCollideDownState();
			break;
		case STATE_COLLIDE_UP:
			EnterCollideUpState();
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
	for (int index = 0; index < MAX_POINTS; index++)
 	{
		Jump();
	}
	std::cout << "Entered Jump State\n";
}

void Ooze::EnterCollideHorizontalState()
{
	std::cout << "Entered CollideHorizontal State\n";
	m_collisionTimer = 0.0f;
}

void Ooze::EnterCollideUpState()
{
	std::cout << "Entered Collide Up State\n";
	m_collisionTimer = 0.0f;
}

void Ooze::EnterCollideDownState()
{
	std::cout << "Entered Collide Down State\n";
	m_collisionTimer = 0.0f;
}


void Ooze::UpdateSprings()
{
	for (int index = 0; index < MAX_SPRINGS; index++)
	{

		float xForce = m_springs[index].b->m_position.x - m_springs[index].a->m_position.x;
		float yForce = m_springs[index].b->m_position.y - m_springs[index].a->m_position.y;
		float magnitudeOfForce = sqrt((xForce * xForce) + (yForce * yForce));
		float displacement = magnitudeOfForce - m_springs[index].currentRestLength;

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

		m_points[index].m_position.x += m_points[index].m_velocity.x;
		m_points[index].m_position.y += m_points[index].m_velocity.y;

		m_points[index].m_acceleration.x = 0;
		m_points[index].m_acceleration.y = 0;	



		m_points[index].lerpTimeElapsedX += t_dt;
		m_points[index].lerpTimeElapsedY += t_dt;

		m_points[index].m_radiusX = Lerp(m_points[index].m_radiusX, m_points[index].m_newRadiusX,
			m_points[index].lerpTimeElapsedX / m_points[index].lerpTime);

		m_points[index].m_radiusY = Lerp(m_points[index].m_radiusY, m_points[index].m_newRadiusY,
			m_points[index].lerpTimeElapsedY / m_points[index].lerpTime);

		float differenceX = m_points[index].m_newRadiusX - m_points[index].m_radiusX;
		float differenceY = m_points[index].m_newRadiusY - m_points[index].m_radiusY;

		if (differenceX < 0.01 && differenceX > -0.01)
		{
			m_points[index].m_newRadiusX = rand() % 20 + 20;
			m_points[index].lerpTimeElapsedX = 0;
		}

		if (differenceY < 0.01 && differenceY > -0.01)
		{
			m_points[index].m_newRadiusY = rand() % 20 + 20;
			m_points[index].lerpTimeElapsedY = 0;
		}
	}

	ClampPointsOnScreen();
}


void Ooze::ClampPointsOnScreen()
{
	for (int index = 0; index < MAX_POINTS; index++)
	{
		if (m_points[index].m_position.x < m_points[index].m_radiusX)
		{

			m_points[index].m_position.x = m_points[index].m_radiusX;
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
			if (fsm.m_currentState == STATE_COLLIDE_DOWN)
				return;
		}	
			
		if (m_points[index].m_position.x > SCREEN_WIDTH - m_points[index].m_radiusX)
		{
			m_points[index].m_position.x = SCREEN_WIDTH - m_points[index].m_radiusX;
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
			if (fsm.m_currentState == STATE_COLLIDE_DOWN)
				return;
		}
			
		if (m_points[index].m_position.y < m_points[index].m_radiusY)
		{
			m_points[index].m_position.y = m_points[index].m_radiusY;
			HandleEvent(EVENT_COLLIDE_UP);
			if (fsm.m_currentState == STATE_COLLIDE_DOWN)
				return;
		}
			
		if (m_points[index].m_position.y > SCREEN_HEIGHT - m_points[index].m_radiusY)
		{
			m_points[index].m_position.y = SCREEN_HEIGHT - m_points[index].m_radiusY;
			HandleEvent(EVENT_COLLIDE_DOWN);
			if (fsm.m_currentState == STATE_COLLIDE_DOWN)
				return;
		}
	}
}

void Ooze::Jump()
{
	float avgRadii = 0.0f;
	for (int index = 0; index < MAX_POINTS; index++)
	{
		avgRadii = (m_points[index].m_radiusX + m_points[index].m_radiusY) / 2;
		m_points[index].m_velocity.y -= (m_jumpAmount * avgRadii / 10);;
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

void Ooze::Spread()
{
	float avgRadii;
	for (int index = 0; index < MAX_POINTS / 2; index++)
	{
		avgRadii = (m_points[index].m_radiusX + m_points[index].m_radiusY) / 2;
		m_points[index].m_velocity.x -= (m_jumpAmount *2 * avgRadii / 5);
	}
	
	for (int index = MAX_POINTS / 2; index < MAX_POINTS; index++)
	{
		float avgRadii;
		m_points[index].m_velocity.x += (m_jumpAmount *2 * avgRadii / 5);
	}


	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].m_newRadiusY = 15;
		m_points[index].m_newRadiusX = 55;
	}

	for (int index = 0; index < MAX_SPRINGS; index++)
	{
		m_springs[index].currentRestLength = m_springs[index].spawnedRestLength + 50; 
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
		// if (index < MAX_POINTS - 1)
		// {
		// 	DrawLineEx(m_points[index].m_position, m_points[index + 1].m_position, m_points[index].m_radius* 1.3, (Color){ 0, 210, 0, 255 });
		// }
	}

	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawEllipse(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radiusX / 1.5,  m_points[index].m_radiusY / 1.5, (Color){ 0, 190, 0, 255 });
		// if (index < MAX_POINTS - 1)
		// {
		// 	DrawLineEx(m_points[index].m_position, m_points[index + 1].m_position, m_points[index].m_radius* 1.3 / 1.5, (Color){ 0, 190, 0, 255 });
		// }
	}

	
	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawEllipse(m_points[index].m_position.x, m_points[index].m_position.y, m_points[index].m_radiusX / 3,  m_points[index].m_radiusY / 3, (Color){ 0, 180, 0, 255 });
	
		// if (index < MAX_POINTS - 1)
		// {
		// 	DrawLineEx(m_points[index].m_position, m_points[index + 1].m_position,  m_points[index].m_radius* 1.3 / 3, (Color){ 0, 180, 0, 255 });
		// }
	

	}


	
}