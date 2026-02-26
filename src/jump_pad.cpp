#include "jump_pad.h"

JumpPad::JumpPad()
{
	m_boundingBox = {0, 0, 0 + m_WIDTH, 0 + m_HEIGHT};
}

void JumpPad::initialize(Vector2 pos)
{
	m_boundingBox = {pos.x, pos.y, m_WIDTH, m_HEIGHT};
}

void JumpPad::update(Ooze &player)
{      
    if (CheckCollisionPointRec(player.CalculateCenter(), m_boundingBox))
    {
		Point *currentPoint;
		for (int index = 0; index < MAX_POINTS; index++)
		{
			currentPoint = &player.GetPoints()[index];
			currentPoint->m_velocity.y -= JUMP_PUSHBACK;
		}

		player.HandleEvent(EVENT_JUMP);
    }    
}

void JumpPad::draw()
{
	DrawRectangle(m_boundingBox.x, m_boundingBox.y, m_WIDTH, m_HEIGHT, VIOLET );
}