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

	cameraSpeed = 6.0f;
}

void CameraManager::begin(){
	BeginMode2D(screen);
}

void CameraManager::end(){
	EndMode2D();
}

void CameraManager::update(Vector2 position){
	if(screen.target.x < position.x){
		screen.target.x += cameraSpeed;
	}
	if(screen.target.x > position.x){
		if(screen.target.x > SCREEN_WIDTH / 2)
		{
			screen.target.x -= cameraSpeed;
		}
	}
	if(screen.target.y < position.y){
		screen.target.y += cameraSpeed;
	}
	if(screen.target.y > position.y){
		if(screen.target.y > SCREEN_HEIGHT / 2)
		{
			screen.target.y -= cameraSpeed;
		}
	}
}