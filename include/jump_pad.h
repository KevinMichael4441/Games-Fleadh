#ifndef JUMP_H
#define JUMP_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "ooze.hpp"

//------------------JumpPad------------------//

class JumpPad
{
public:
	JumpPad();
	void Initialize(Vector2 pos);

	void Update(Ooze &player);
	void Draw();

private:
	Rectangle m_boundingBox;
	static const int m_WIDTH = 32;
	static const int m_HEIGHT = 32;
};

//------------------Manager------------------//

class JumpPad_Manager
{
public:
	JumpPad_Manager();
	void Initialize(Vector2 pos);

	void Update(Ooze &player);
	void Draw();

private:
	JumpPad m_jumpPadds[MAX_JUMPPADS];
};

#endif