#include "jump_pad.h"

//------------------JumpPad------------------//

JumpPad::JumpPad()
{
	m_boundingBox = {0, 0, 0 + m_WIDTH, 0 + m_HEIGHT};
}

void JumpPad::Initialize(Vector2 pos)
{
	m_boundingBox = {pos.x, pos.y, m_WIDTH, m_HEIGHT};
}

void JumpPad::Update(Ooze &player)
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

void JumpPad::Draw()
{
	DrawRectangle(m_boundingBox.x, m_boundingBox.y, m_WIDTH, m_HEIGHT, VIOLET );
}

//------------------Manager------------------//

JumpPad_Manager::JumpPad_Manager()
{
}

void JumpPad_Manager::Initialize(Vector2 pos)
{
	for (int index = 0; index < MAX_JUMPPADS; index++)
	{
		m_jumpPadds[index].Initialize({672,384});
	}
}

void JumpPad_Manager::Update(Ooze &player)
{      
    for (int index = 0; index < MAX_JUMPPADS; index++)
	{
		m_jumpPadds[index].Update(player);
	}
}

void JumpPad_Manager::Draw()
{
	for (int index = 0; index < MAX_JUMPPADS; index++)
	{
		m_jumpPadds[index].Draw();
	}
}