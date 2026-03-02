//#ifndef CAMTYPE_H
//#define CAMTYPE_H
#pragma once

enum CamType
{
	CAM_SPOT,
	CAM_SWEEP
};

enum CamMount
{
    MOUNT_BACKGROUND = 0,
    MOUNT_LEFT_WALL = 1,
	MOUNT_RIGHT_WALL = 2,
    MOUNT_CEILING = 3
};

enum LaserDir
{
    LASER_N = 0,
    LASER_E = 1,
    LASER_S = 2,
    LASER_W = 3
};

//#endif