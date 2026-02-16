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
	if(slimePos.x < screen.target.x - WINDOW)
	{
		if(direction.x > -1.0f)
		{
			direction.x -= increment;
		}
	}
	if(slimePos.x > screen.target.x + WINDOW)
	{
		if(direction.x < 1.0f)
		{
			direction.x += increment;
		}
	}
	if(slimePos.y < screen.target.y - WINDOW)
	{
		if(direction.y > -1.0f)
		{
			direction.y -= increment;
		}
	}
	if(slimePos.y > screen.target.y + WINDOW)
	{
		if(direction.y < 1.0f)
		{
			direction.y += increment;
		}
	}

	if(slimePos.x > screen.target.x - WINDOW &&
	   slimePos.x < screen.target.x + WINDOW)
	{
		direction.x = 0.0f;
	}

	if(slimePos.y > screen.target.y - WINDOW &&
	   slimePos.y < screen.target.y + WINDOW)
	{
		direction.y = 0.0f;
	}
}

void CameraManager::move()
{
	Vector2 newPos = screen.target;

	if(newPos.x >= SCREEN_WIDTH / 2)
	{
		newPos.x += direction.x * speed;
	}
	if(newPos.y >= SCREEN_HEIGHT / 2)
	{
		newPos.y += direction.y * vertSpeed;
	}
	if(newPos.x < SCREEN_WIDTH / 2)
	{
		newPos.x = SCREEN_WIDTH / 2;
	}
	if(newPos.y < SCREEN_HEIGHT / 2)
	{
		newPos.y = SCREEN_HEIGHT / 2;
	}

	screen.target = newPos;
}