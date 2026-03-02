#ifndef JUMP_H
#define JUMP_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"
#include "level_loader.h"
#include <cstring>

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
	void Initialize(LevelData* level);

	void Update(Ooze &player);
	void Draw();

private:
	JumpPad m_jumpPads[MAX_JUMPPADS];
	int m_jumpPadCount = 0;
};

#endif