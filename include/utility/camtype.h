#ifndef CAMTYPE_H
#define CAMTYPE_H

typedef enum CamType
{
	CAM_NONE,
	CAM_SPOT,
	CAM_SWEEP
} CamType;

typedef enum CamDirection
{
	N,
	S,
	E,
	W,
	NE,
	NW,
	SE,
	SW
} CamDirection;

#endif