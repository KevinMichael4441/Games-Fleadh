#include "game.h"

void InitGame(GameData *data)
{
	data->springConstant = 0.01;
	data->damp = 0.95;

	data->centrePoint = (Vector2){0,0};


	for (int index = 0; index < MAX_POINTS; index++)
	{
		Point point = {
			{0,0},
			{0,0},
			{rand() % 30 + SCREEN_WIDTH/2 - 15, rand() % 30 + SCREEN_HEIGHT/2 - 15},
			false,
			(rand() % 20 + 20),
			(rand() % 20 + 20),
			0.0f,
			BLACK,
			false,
			0.0f,
			7.5f
		};

		point.m_mass = point.m_radius * point.m_radius;

		data->points[index] = point;
	}


	for (int currentPoint = 0; currentPoint < MAX_POINTS; currentPoint++)
	{
		for (int nextPoint = currentPoint + 1 ; nextPoint < MAX_POINTS; nextPoint++)
		{
			Point *a = &data->points[currentPoint];
			Point *b = &data->points[nextPoint];

			float xDistance = {b->m_position.x - a->m_position.x};
			float yDistance = {b->m_position.y - a->m_position.y};
			float magnitudeOfDistance = sqrt((xDistance * xDistance) + (yDistance * yDistance));
			Spring spring =  { data->springConstant, magnitudeOfDistance, a, b} ;


			switch (currentPoint)
			{
				case 0:
					data->springs[nextPoint - 1] = spring;
					break;
				case 1:
					data->springs[nextPoint + 4] = spring;
					break;
				case 2:
					data->springs[nextPoint + 8] = spring;
					break;
				case 3:
					data->springs[nextPoint + 11] = spring;
					break;
				case 4:
					data->springs[nextPoint + 13] = spring;
					break;
				case 5:
					data->springs[nextPoint + 14] = spring;
					break;			
			}
		}	
   	}


	
	data->gravity.x = 0;
	data->gravity.y = 0.1;

	data->speed = 0.5;
	data->jumpAmount = 0.8;

	data->spawnTimer = 0.0f;
	data->spawnDelay = 0.1f;

	for (int index = 0; index < MAX_BEZIER_POINTS; index ++)
	{
		data->bezierPoints[index] = (Vector2){0,0};
		data->bezierMoving[index] = false;
	}
}

void resetBlob(GameData* data)
{
	Vector2 center = {0,0};
	float radius = 1;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		float x = center.x + (radius * (cos(2 * index * PI / MAX_POINTS)));
		float y = center.y + (radius * (sin(2 * index * PI / MAX_POINTS)));

		data->points[index].m_acceleration.x += x;
		data->points[index].m_acceleration.y += y;
	}
}

void UpdateGame(GameData *data, float deltaTime)
{

	if (data->inputRecieved)
	{
		if (data->relapseTimer > RELAPSE_DELAY)
		{
			resetBlob(data);
			data->relapseTimer = 0.0f;
			data->inputRecieved = false;
		}
		else
		{
			data->relapseTimer += deltaTime;
		}
	}


	//if (data->isjumping)
	//{
	//	data->jumpTimer+= deltaTime;
	//	if (data->jumpTimer >= data->jumpDelay)
	//	{
	//		data->isjumping = false;
	//	}
	//}

	updateSprings(data, deltaTime);
	//updateMasterPoint(data, deltaTime);
	updatePoints(data, deltaTime);
	updateBezierPoints(data, deltaTime);
}

//void updateMasterPoint(GameData* data, float deltaTime)
//{
// 	(void)deltaTime;
// 	if (IsKeyDown(KEY_LEFT))
// 	{
// 		data->masterPoint.m_velocity.x -= data->speed;
// 		data->inputRecieved = true;
// 	}	
		
// 	if (IsKeyDown(KEY_RIGHT))
// 	{
// 		data->masterPoint.m_velocity.x += data->speed;
// 		data->inputRecieved = true;
// 	}

// 	if (IsKeyPressed(KEY_SPACE))
// 	{
// 		jump(data);
// 	}

// 	data->masterPoint.m_acceleration.x += data->gravity.x;
// 	data->masterPoint.m_acceleration.y += data->gravity.y;

// 	data->masterPoint.m_velocity.x += data->masterPoint.m_acceleration.x;
// 	data->masterPoint.m_velocity.y += data->masterPoint.m_acceleration.y;

// 	data->masterPoint.m_velocity.x *= data->damp;
// 	data->masterPoint.m_velocity.y *= data->damp;


// 	data->masterPoint.m_position.x +=data->masterPoint.m_velocity.x;
// 	data->masterPoint.m_position.y +=data->masterPoint.m_velocity.y;

// 	data->masterPoint.m_acceleration.x = 0;
// 	data->masterPoint.m_acceleration.y = 0;
// }

void jumpActual(GameData* data)
{
	//data->masterPoint.m_velocity.y -= data->jumpAmount;
	data->inputRecieved = true;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		data->points[index].m_velocity.y -= (data->jumpAmount * data->points[index].m_radius / 10);;
	}

	//data->isjumping = true;
	//data->jumpTimer = 0.0f;
}


void jump(GameData* data)
{
	//data->masterPoint.m_velocity.y -= data->jumpAmount;
	data->inputRecieved = true;

	//for (int index = 0; index < MAX_POINTS; index++)
	//{
	//	data->points[index].m_velocity.y -= (data->jumpAmount * data->points[index].m_radius / 10);;
	//}

	for (int index = 0; index < MAX_POINTS / 2; index++)
	{
		data->points[index].m_velocity.x -= (data->jumpAmount * data->points[index].m_radius / 10);;
	}
	
	for (int index = MAX_POINTS / 2; index < MAX_POINTS; index++)
	{
		data->points[index].m_velocity.x += (data->jumpAmount * data->points[index].m_radius / 10);;
	}

	//data->isjumping = true;
	//data->jumpTimer = 0.0f;
}

void updateBezierPoints(GameData* data, float deltaTime)
{

	
	(void)deltaTime;

	makePointForBezierFromCircle(data);
	// 0,2,4,6,8,10,12, 14

	for (int index = 1; index < MAX_BEZIER_POINTS - 1; index += 2)
	//1, 3, 5, 7, 9, 11, 13
	{
		Vector2 controlPoint;
		controlPoint  = calculateControlPoint(data->bezierPoints[index - 1], data->bezierPoints[index + 1], data->centrePoint);
		data->bezierPoints[index] = controlPoint;
	}	

	if (data->bezierTimer > data->bezierDelay)
	{
		//data->bezierTimer = 0.0f;
		//randomizePoints(data);
	}

}

void makePointForBezierFromCircle(GameData* data)
{
	// Calculate the center point of figure
	// 1 - loop through points to get minx, maxX, minY, maxY

	float minX = SCREEN_WIDTH;
	float maxX = 0.0f;;
	float minY = SCREEN_HEIGHT;
	float maxY = 0.0f;
	
	for (int index = 0; index < MAX_POINTS; index++)
	{
		if (data->points[index].m_position.x < minX)
		{
			minX = data->points[index].m_position.x;
		}
		
		if (data->points[index].m_position.x > maxX)
		{
			maxX = data->points[index].m_position.x;
		}

		if (data->points[index].m_position.y < minY)
		{
			minY = data->points[index].m_position.y;
		}
		
		if (data->points[index].m_position.y > maxY)
		{
			maxY = data->points[index].m_position.y;
		}
	}

	data->centrePoint = (Vector2){(minX + maxX) / 2, (minY + maxY) / 2};

	for (int index = 0; index < MAX_POINTS; index++)
	{
		float deltaX = data->points[index].m_position.x - data->centrePoint.x;
		float deltaY = data->points[index].m_position.y - data->centrePoint.y;

		float angle = atan2f(deltaY, deltaX);
		angle -= 1.5708;	// subtracting 90 degrees

		float afterOffset = (data->points[index].m_radius + rand() % 10 + 5);

		Vector2 pointOnCurve = { afterOffset * sin(angle), afterOffset  * cos(angle)};
		data->bezierPoints[index * 2] = pointOnCurve;
	}

	data->bezierPoints[MAX_BEZIER_POINTS - 1] = data->bezierPoints[0]; 
}




Vector2 calculateControlPoint(Vector2 t_m, Vector2 t_n, Vector2 t_centrePoint)
{
	(void)t_centrePoint;
	float x = (t_m.x + t_n.x) / (float)2;		// midpoint x
	float y = (t_m.y + t_n.y) / (float)2;		// midpoint y

	//float deltaX = x - t_centrePoint.x;
	//float deltaY = y - t_centrePoint.y;

	float offset = 10;

	Vector2 controlPoint = {x - offset * sin(90), y + offset * cos(90)};

	return controlPoint;
}

void updateSprings(GameData *data, float deltaTime) 
{
	(void) deltaTime;

	for (int index = 0; index < MAX_SPRINGS; index++)
	{

		float xForce = data->springs[index].b->m_position.x - data->springs[index].a->m_position.x;
		float yForce = data->springs[index].b->m_position.y - data->springs[index].a->m_position.y;

		float magnitudeOfForce = sqrt((xForce * xForce) + (yForce * yForce));

		float displacement = magnitudeOfForce - data->springs[index].restLength;

		//if (data->springs[index].b->m_lock || data->springs[index].b->m_lock)
		//{
		//	displacement = magnitudeOfForce - data->springs[index].restLength;
		//}
    	//else
		//{
		//	displacement = magnitudeOfForce - data->springs[index].restLength;
		//}

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
		
		normalizedForce.x *= data->springs[index].springConstant * displacement;
		normalizedForce.y *= data->springs[index].springConstant * displacement;

		float sumOfMass = data->springs[index].a->m_mass + data->springs[index].b->m_mass;
		float weightOfA = data->springs[index].b->m_mass / sumOfMass;
		float weightOfB = data->springs[index].a->m_mass / sumOfMass;
	
		if (!data->springs[index].a->m_lock)//|| (data->isjumping && data->springs[index].a != &data->masterPoint))
		{
			data->springs[index].a->m_acceleration.x += normalizedForce.x * weightOfA;
			data->springs[index].a->m_acceleration.y += normalizedForce.y * weightOfA;
		}


		normalizedForce.x *= -1;
		normalizedForce.y *= -1;
    	
		if (!data->springs[index].b->m_lock)
		{
			data->springs[index].b->m_acceleration.x += normalizedForce.x * weightOfB;
			data->springs[index].b->m_acceleration.y += normalizedForce.y * weightOfB;
		}
		
	}
	

}

void updatePoints(GameData *data, float deltaTime)
{

	for (int index = 0; index < MAX_POINTS; index++)
	{

		//if (IsKeyDown(KEY_Z) && !data->pointsLocked)
		//{
		//	lockPoints(data);
		//	data->pointsLocked = true;
		//	data->inputRecieved = true;
		//}

		//if (IsKeyReleased(KEY_Z))
		//{
		//	unlockPoints(data);
		//	data->pointsLocked = false;
		//}

		if (IsKeyDown(KEY_LEFT))
		{
			data->points[index].m_velocity.x -= (data->speed * data->points[index].m_radius / 10);
			data->inputRecieved = true;
		}	
		
		if (IsKeyDown(KEY_RIGHT))
		{
			data->points[index].m_velocity.x += (data->speed * data->points[index].m_radius / 10);
			data->inputRecieved = true;
		}

		if (IsKeyPressed((KEY_X)))
		{
			jump(data);
		}
		
		if (IsKeyPressed(KEY_Z))
		{
			jumpActual(data);
		}

		if (!data->points[index].m_lock)
		{
			data->points[index].m_acceleration.y += (data->gravity.y * data->points[index].m_radius / 10);

			data->points[index].m_velocity.x *= data->damp;
			data->points[index].m_velocity.y *= data->damp;
	
			data->points[index].m_velocity.x += data->points[index].m_acceleration.x;
			data->points[index].m_velocity.y += data->points[index].m_acceleration.y;

			data->points[index].m_position.x += data->points[index].m_velocity.x;
			data->points[index].m_position.y += data->points[index].m_velocity.y;

			ClampPlayerOnScreen(data, deltaTime, index);

			data->points[index].m_acceleration.x = 0;
			data->points[index].m_acceleration.y = 0;
		}



		data->points[index].lerpTimeElapsed += deltaTime;
		data->points[index].m_radius = Lerp(data->points[index].m_radius, data->points[index].m_newRadius, data->points[index].lerpTimeElapsed / data->points[index].lerpTime);

		float difference = data->points[index].m_newRadius - data->points[index].m_radius;
		if (difference < 0.01 && difference > -0.01)
		{
			data->points[index].m_newRadius = rand() % 20 + 20;
			data->points[index].lerpTimeElapsed = 0;
		}
		
		//printf("index: %d ", index);
		//printf("x: %f ", data->points[index].m_position.x);
		//printf("y: %f\n", data->points[index].m_position.y);
	}

	//for (int index = 0; index < MAX_POINTS + 1; index++)
	//{
	//	if (index == MAX_POINTS)
	//	{
	//		data->pointCoords[index] = data->points[0].m_position;
	//	}
	//	else
	//	{
	//		data->pointCoords[index] = data->points[index].m_position;
	//	}
	//}

}

void lockPoints(GameData *data)
{
	if (data->pointsLocked)
	{
		return;
	}

	for (int index = 0; index < MAX_POINTS; index++)
	{
		float pointMinX = data->points[index].m_position.x - data->points[index].m_radius;
		float pointMaxX = data->points[index].m_position.x + data->points[index].m_radius;
		float pointMaxY = data->points[index].m_position.y + data->points[index].m_radius;
		
	    if (pointMinX <= SCREEN_WIDTH &&
    		pointMaxX >= 0 &&
    		pointMaxY >= SCREEN_HEIGHT )
			{
				data->points[index].m_lock = true;
			}
	}
}

void unlockPoints(GameData *data)
{
	for (int index = 0;  index < MAX_POINTS; index++)
	{
		data->points[index].m_lock = false;
	}
}

void ClampPlayerOnScreen(GameData *data, float deltatime, int index)
{
	//Vector2 center = calculateCenter(data);

	(void) deltatime;
		if (data->points[index].m_position.x < data->points[index].m_radius)
		{
			data->points[index].m_position.x = data->points[index].m_radius;
			//data->points[index].m_velocity.x = -1*data->points[index].m_velocity.x;
		}
			
			
		if (data->points[index].m_position.x > SCREEN_WIDTH - data->points[index].m_radius)
		{
			data->points[index].m_position.x = SCREEN_WIDTH - data->points[index].m_radius;
			//data->points[index].m_velocity.x = -1*data->points[index].m_velocity.x;
		}
			

		if (data->points[index].m_position.y < data->points[index].m_radius)
		{
			data->points[index].m_position.y = data->points[index].m_radius;
			//data->points[index].m_velocity.y = -1*data->points[index].m_velocity.y;
		}
			

		if (data->points[index].m_position.y > SCREEN_HEIGHT - data->points[index].m_radius)
		{
			data->points[index].m_position.y = SCREEN_HEIGHT - data->points[index].m_radius;
			

			//if (data->points[index].m_position.x < center.x)
			//{
			//	data->points[index].m_velocity.x -= data->points[index].m_velocity.y;
			//}
			//else
			//{
			//	data->points[index].m_velocity.x += data->points[index].m_velocity.y;
			//}

			//data->points[index].m_velocity.y = 0;
		}
}


Vector2 calculateCenter(GameData* data)
{
	float sumX = 0.0f;
	float sumY = 0.0f;

	for (int index = 0; index < MAX_POINTS; index++)
	{
		sumX += data->points[index].m_position.x;
		sumY += data->points[index].m_position.y;
	}

	float x = sumX / MAX_POINTS;
	float y = sumY / MAX_POINTS;
	
	return (Vector2){x,y};
}




void DrawGame(GameData *data)
{
	//ssssDrawSplineBezierQuadratic(data->bezierPoints, MAX_BEZIER_POINTS, 1, BLACK);
	
	//DrawCircle(data->masterPoint.m_position.x, data->masterPoint.m_position.y, 1, BLUE);

	//for (int index = 0; index < MAX_BEZIER_POINTS; index++)
	//{
	//	DrawCircle(data->bezierPoints[index].x, data->bezierPoints[index].y, 1, RED);
	//}


	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(data->points[index].m_position.x, data->points[index].m_position.y, data->points[index].m_radius, (Color){ 0, 210, 0, 255 });
	}

	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(data->points[index].m_position.x, data->points[index].m_position.y, data->points[index].m_radius / 1.5, (Color){ 0, 190, 0, 255 });
	}

	
	for (int index = 0; index < MAX_POINTS; index++)
	{
		DrawCircle(data->points[index].m_position.x, data->points[index].m_position.y, data->points[index].m_radius / 3, (Color){ 0, 180, 0, 255 });
	}

	//for (int index = 1; index < MAX_SPRINGS; index++)
	//{
	//	if (index > 20)
	//	{
	//		continue;
	//	}
	//	DrawLineEx(data->springs[index].a->m_position, data->springs[index].b->m_position, 12, GREEN);
	//}

	//for (int index = 0; index < MAX_POINTS; index++)
	//{
	//	if (index == MAX_POINTS - 1)
	//	{
	//		DrawLineBezier(data->points[index].m_position, data->points[0].m_position, 5, GREEN);
	//	}
	//	else
	//	{
	//		DrawLineBezier(data->points[index].m_position, data->points[index+1].m_position, 5, GREEN);
	//	}
	//}



}

void CloseGame(GameData *data)
{
	free(data); // Free the allocated memory for GameData
	
	printf("Game Closed!\n");
}