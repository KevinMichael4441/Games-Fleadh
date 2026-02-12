#ifndef CAMERA_MANAGER_HPP
#define CAMERA_MANAGER_HPP

#include <raylib.h>
#include <stdio.h>
#include "constants.h"

class CameraManager
{

	public:
		CameraManager();
		~CameraManager();

		void update(Vector2 position);
		void begin();
		void end();

	private:
		void initialize();

		float cameraSpeed;
		Camera2D screen;
		
};

#endif