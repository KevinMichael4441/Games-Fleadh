#ifndef JUMP_H
#define JUMP_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "ooze.hpp"

class JumpPad
{
public:
	JumpPad();
	void initialize(Vector2 pos);

	void update(Ooze &player);
	void draw();

private:
	Rectangle m_boundingBox;
	static const int m_WIDTH = 32;
	static const int m_HEIGHT = 32;
};

#endif