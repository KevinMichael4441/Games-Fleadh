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
}

void CameraManager::begin(){
	BeginMode2D(screen);
}

void CameraManager::end(){
	EndMode2D();
}

void CameraManager::updateCamCenter(Vector2& t_position)
{
	screen.offset = (Vector2){ (SCREEN_WIDTH/2.0f), (SCREEN_HEIGHT/2.0f)};
	screen.target = (Vector2){t_position.x, t_position.y};
}

void CameraManager::moveCamInsideMap(Vector2& t_position)
{
	float minX = 0.0f;
	float minY = 0.0f;
	float maxX = 1280.0f;
	float maxY = 960.0f;

	Vector2 max = GetWorldToScreen2D((Vector2){ maxX, maxY }, screen);
    Vector2 min = GetWorldToScreen2D((Vector2){ minX, minY }, screen);

	if (max.x < SCREEN_WIDTH) screen.offset.x = SCREEN_WIDTH - (max.x - (float)SCREEN_WIDTH/2);
    if (max.y < SCREEN_HEIGHT) screen.offset.y = SCREEN_HEIGHT - (max.y - (float)SCREEN_HEIGHT/2);
    if (min.x > 0) screen.offset.x = (float)SCREEN_WIDTH/2 - min.x;
    if (min.y > 0) screen.offset.y = (float)SCREEN_HEIGHT/2 - min.y;
}

void CameraManager::update(Vector2 t_position)
{
	updateCamCenter(t_position);
	moveCamInsideMap(t_position);
}