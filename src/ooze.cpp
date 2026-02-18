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


	m_gravity = {0.0f, 0.1f};

	for (int index = 0; index < MAX_POINTS; index++)
	{
		Point point = {
			{0,0},
			{0,0},
			{(float)(80), (float)(rand() % 30 + SCREEN_HEIGHT/2 - 15)},

			((float)(rand() % 20 + 20)),
			((float)(rand() % 20 + 20)),

			((float)(rand() % 20 + 20)),
			((float)(rand() % 20 + 20)),
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

	if (jumping && 
		(fsm.m_currentState == STATE_MOVING || fsm.m_currentState == STATE_IDLE))
	{
		for (int index = 0; index < MAX_POINTS; index++)
 		{
			Jump();
		}
		HandleEvent(EVENT_JUMP);
	}
}	

void Ooze::HandleEvent(Event t_event)
{
	fsm.m_previousState = fsm.m_currentState;
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
		return;
	}

	DefaultUpdate(t_dt);
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
			m_points[index].m_newRadiusX = rand() % 20 + 20;
			m_points[index].m_newRadiusY = rand() % 20 + 20;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		// X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 10 + 30;
			m_points[index].m_newRadiusY = rand() % 10 + 25;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 20 + 60;
			m_points[index].m_newRadiusY = rand() % 5 + 15;
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
			m_points[index].m_newRadiusX = rand() % 20 + 20;
			m_points[index].m_newRadiusY = rand() % 20 + 20;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		// X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 10 + 40;
			m_points[index].m_newRadiusY = rand() % 5 + 25;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 20 + 80;
			m_points[index].m_newRadiusY = rand() % 5 + 10;
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
		// X and Y reset back
		// Circles pushed down
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 20 + 20;
			m_points[index].m_newRadiusY = rand() % 20 + 20;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[3] = true;
	}
	else if (m_collisionTimer > 0.4f && !collisionParts[2])
	{
		// X is slightly more than normal
		// Y is higher
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 5 + 25;
			m_points[index].m_newRadiusY = rand() % 10 + 25;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[2] = true;
	}
	else if (m_collisionTimer > 0.2f && !collisionParts[1])
	{
		// X is reducing and Y is increasing
		//calculateExtremePos();
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 10 + 50;
			m_points[index].m_newRadiusY = rand() % 10 + 20;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}

		collisionParts[1] = true;
	}
	else if (!collisionParts[0])
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_newRadiusX = rand() % 20 + 100;
			m_points[index].m_newRadiusY = 5;
			m_points[index].lerpTimeElapsedX = 0;
			m_points[index].lerpTimeElapsedY = 0;
		}
		
		collisionParts[0] = true;
	}
}

void Ooze::UpdateCollideUpState(float t_dt)
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
	}

	DefaultUpdate(t_dt);
	Move();
}

void Ooze::UpdateCollideDownState(float t_dt)
{
	m_collisionTimer += t_dt;

	if (m_collisionTimer > 1.0f)
	{
		HandleEvent(EVENT_TIMER);
		return;
	}

	DefaultUpdate(t_dt);

	//Spread();
}

void Ooze::DefaultUpdate(float t_dt)
{
	UpdateSprings();
	UpdatePoints(t_dt);
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

	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].lerpTime = 7.5f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

	if (CalculateCenter().x > SCREEN_WIDTH / 2)
	// If collided on right side
	// MUST REFACTOR WHEN LEVEL LOADING IS DONE
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.x -= 2.0f; 
		}
	}
	else
	{
		for (int index = 0; index < MAX_POINTS; index++)
		{
			m_points[index].m_velocity.x += 2.0f; 
		}
	}
}

void Ooze::ExitCollideUpState()
{
	std::cout << "Exited Collide Up State\n";

	// reset Collision Timer
	m_collisionTimer = 0.0f;

	// Reset lerpTime and lerpElapsedTime
	// Also reset extremePos flag
	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].lerpTime = 7.5f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

	
	// RESET REST LENGTH

	// reset collision part flag
	for (int index = 0; index < MAX_COLLISION_PARTS; index++)
	{
		collisionParts[index] = false;
	}
}

void Ooze::ExitCollideDownState()
{
	std::cout << "Exited Collide Down State\n";
	m_collisionTimer = 0.0f;
	for (int index = 0; index < MAX_POINTS; index++)
	{
		m_points[index].lerpTime = 7.5f;
		m_points[index].lerpTimeElapsedX = 0;
		m_points[index].lerpTimeElapsedY = 0;
	}

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
	std::cout << "Entered Jump State\n";
}

void Ooze::EnterCollideHorizontalState()
{
	std::cout << "Entered CollideHorizontal State\n";
	m_collisionTimer = 0.0f;

	// for(int index = 0; index < MAX_POINTS; index++)
	// {
	// 	float velocityMagnitude = abs(m_points[index].m_velocity.x);

	// 	if (velocityMagnitude > 100)
	// 	{
	// 		m_points[index].m_newRadiusY = rand() % 20 + 60;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].m_newRadiusX = rand() %  5 + 15;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].lerpTime = 0.4;
	// 	}
	// 	else if (velocityMagnitude > 50)
	// 	{
	// 		m_points[index].m_newRadiusY = rand() % 10 + 40;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].m_newRadiusX = rand() % 5 + 20;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].lerpTime = 0.8;
	// 	}
	// 	else if (velocityMagnitude > 50)
	// 	{
	// 		m_points[index].m_newRadiusY = rand() % 10 + 30;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].m_newRadiusX = rand() % 5 + 25;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].lerpTime = 1.2;
	// 	}

	// }
}

void Ooze::EnterCollideUpState()
{
	std::cout << "Entered Collide Up State\n";
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

	if (avgVelocity < 20)
	{
		HandleEvent(EVENT_TIMER);
		return;
	}
	else if (avgVelocity < 40)
	{
		m_squishiness = SQUISH_AMOUNT::LOW;
	}
	else if (avgVelocity < 60)
	{
		m_squishiness = SQUISH_AMOUNT::MEDIUM;
	}
	else 
	{
		m_squishiness = SQUISH_AMOUNT::HIGH;
	}
}

void Ooze::EnterCollideDownState()
{
	std::cout << "Entered Collide Down State\n";
	m_collisionTimer = 0.0f;

	// for(int index = 0; index < MAX_POINTS; index++)
	// {
	// 	float velocityMagnitude = abs(m_points[index].m_velocity.y);
	// 	if (velocityMagnitude > 20)
	// 	{
	// 		m_points[index].m_newRadiusX = rand() % 20 + 60;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].m_newRadiusY = rand() %  5 + 15;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].lerpTime = 0.4;
	// 	}
	// 	else if (velocityMagnitude > 10)
	// 	{
	// 		m_points[index].m_newRadiusX = rand() % 10 + 40;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].m_newRadiusY = rand() % 5 + 20;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].lerpTime = 0.8;
	// 	}
	// 	else if (velocityMagnitude < 5)
	// 	{
	// 		m_points[index].m_newRadiusX = rand() % 10 + 30;
	// 		m_points[index].lerpTimeElapsedX = 0;

	// 		m_points[index].m_newRadiusY = rand() % 5 + 25;
	// 		m_points[index].lerpTimeElapsedY = 0;

	// 		m_points[index].lerpTime = 1.2;
	// 	}
	// }

	
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


		if (fsm.m_currentState == STATE_IDLE || 
			fsm.m_currentState == STATE_JUMPING ||
			fsm.m_currentState == STATE_MOVING)
		{
			SetNewLerp(index, 20,20,20,20);	
		}
		
	}

	ClampPointsOnScreen();
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


void Ooze::ClampPointsOnScreen()
{
	for (int index = 0; index < MAX_POINTS; index++)
	{
		if (m_points[index].m_position.x < m_points[index].m_radiusX)
		{

			m_points[index].m_position.x = m_points[index].m_radiusX;
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
		}	
			
		if (m_points[index].m_position.x > SCREEN_WIDTH - m_points[index].m_radiusX)
		{
			m_points[index].m_position.x = SCREEN_WIDTH - m_points[index].m_radiusX;
			HandleEvent(EVENT_COLLIDE_HORIZONTAL);
		}
			
		if (m_points[index].m_position.y < m_points[index].m_radiusY)
		{
			m_points[index].m_position.y = m_points[index].m_radiusY;
			HandleEvent(EVENT_COLLIDE_UP);
			//m_points[index].m_velocity.y = 0.0f;
		}
			
		if (m_points[index].m_position.y > SCREEN_HEIGHT - m_points[index].m_radiusY)
		{
			m_points[index].m_position.y = SCREEN_HEIGHT - m_points[index].m_radiusY;
			HandleEvent(EVENT_COLLIDE_DOWN);
			//m_points[index].m_velocity.y = 0.0f;
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