#ifndef CAMERA_MANAGER_HPP
#define CAMERA_MANAGER_HPP

#include <raylib.h>
#include <stdio.h>
#include "constants.h"
#include "command.h"

typedef enum CamState
{
	STATIC,
	MOVING
} CamState;

class CameraManager
{

	public:
		CameraManager();
		~CameraManager();

		void begin();
		void end();

		void update(Vector2 position);

	private:
		void initialize();
		void move();
		void updateDirection();



		Camera2D screen;

		CamState state;

		Vector2 slimePos;
		Vector2 direction;

		float speed = 10.0f;
		float offset = 50.0f;
		float WINDOW = 30.0f; // Space around slime before camera starts to move
};

#endif