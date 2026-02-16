#include "camera_manager.hpp"

CameraManager::CameraManager(){
	initialize();
	printf("Camera Manager object created\n");
}

CameraManager::~CameraManager(){
	printf("Camera Manager object destroyed\n");
}

void CameraManager::initialize(){
	screen.offset = Vector2({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2}); // Window origin in screen space
	screen.target = Vector2({SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});; // Window origin position in World Space
	screen.rotation = 0.0f; // Degrees
	screen.zoom = 1.0f; // Scale

	state = MOVING;

	slimePos = {0.0f,0.0f};
	direction = {0.0f,0.0f};
}

void CameraManager::begin(){
	BeginMode2D(screen);
}

void CameraManager::end(){
	EndMode2D();
}

void CameraManager::update(Vector2 position)
{
	slimePos = position;
	updateDirection();
	move();
}

void CameraManager::updateDirection()
{
	if(screen.target.x > SCREEN_WIDTH / 2)
	{
		if(slimePos.x < screen.target.x - WINDOW)
		{
			if(direction.x > -1.0f)
			{
				direction.x -= 0.05f;
			}
		}
	}
	if(screen.target.x <= (SCREEN_WIDTH / 2) + 5)
	{
		direction.x = 0.0f;
	}

	if(slimePos.x > screen.target.x + WINDOW)
	{
		if(direction.x < 1.0f)
		{
			direction.x += 0.05f;
		}
	}

	if(screen.target.y > SCREEN_HEIGHT)
	{
		if(slimePos.y < screen.target.y - WINDOW)
		{
			if(direction.y > -1.0f)
			{
				direction.y -= 0.05f;
			}
		}
	}
	if(screen.target.y <= (SCREEN_HEIGHT / 2) + 5)
	{
		direction.y = 0.0f;
	}

	if(slimePos.y > screen.target.y + WINDOW)
	{
		if(direction.y < 1.0f)
		{
			direction.y += 0.05f;
		}
	}
}

void CameraManager::move()
{
	if(screen.target.x < slimePos.x - WINDOW || screen.target.x > slimePos.x + WINDOW)
	{
		screen.target.x += direction.x * speed;
	}
	else
	{
		direction.x = 0.0f;
	}

	if(screen.target.y < slimePos.y - (WINDOW / 2) || screen.target.y > slimePos.y + (WINDOW / 2))
	{
		screen.target.y += direction.y * speed;
	}
}