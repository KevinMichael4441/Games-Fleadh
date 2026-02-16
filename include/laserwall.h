#ifndef LASER_H
#define LASER_H


#include <raylib.h>
#include <raymath.h>

#include "constants.h"


class Laserwall
{
public:
	Laserwall();
	void initialize(float t_x, float t_y);
	void update(float t_dt);
	void draw();


private:

	static const int MAX_BARS = 6;
	static const int m_BAR_WIDTH = 4;
	static const int m_BAR_HEIGHT = 128;
	static const int m_WIDTH = 64;
	static const int m_HEIGHT = 128;

	Rectangle m_bars[MAX_BARS];
	Rectangle m_boundingBox;


	float m_timer;
	float m_delay;

	bool m_isActive;

};


#endif