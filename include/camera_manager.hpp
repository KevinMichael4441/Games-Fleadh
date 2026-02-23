#ifndef CAMERA_MANAGER_HPP
#define CAMERA_MANAGER_HPP

#include <raylib.h>
#include <stdio.h>

#include <math.h>

#include "constants.h"
#include "command.h"
#include <stdlib.h>
#include "math.h"

typedef enum CamMode
{
	EXACT,
	FOLLOW,
	DEADZONE
} CamMode;

class CameraManager
{
	public:
		CameraManager();
		~CameraManager();

		void begin();
		void end();

		void update(Vector2 positionCommand);

		Camera2D screen;
	private:
		void initialize();

		void updateCamCenter(Vector2& t_position);
		void moveCamInsideMap(Vector2& t_position);

	
		CamMode mode;

		float WINDOW = 70.0f; // Space around slime before camera starts to move
};

#endif