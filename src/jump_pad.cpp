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
	m_jumpPadCount = 0;
}

void JumpPad_Manager::Initialize(LevelData* level)
{
    m_jumpPadCount = 0;

    if (!level || !level->objects || level->objectCount <= 0)
	{
		return;
	}

    for (int i = 0; i < level->objectCount; i++)
    {
        const LevelObject& obj = level->objects[i];

        if (!obj.type || strcmp(obj.type, "JumpPad") != 0)
		{
			continue;
		}

        if (m_jumpPadCount >= MAX_JUMPPADS)
		{
			break;
		}

        m_jumpPads[m_jumpPadCount].Initialize({ obj.x, obj.y });
        m_jumpPadCount++;
    }

    TraceLog(LOG_INFO, "Spawned %d jump pads", m_jumpPadCount);
}

void JumpPad_Manager::Update(Ooze &player)
{      
    for (int i = 0; i < m_jumpPadCount; i++)
	{
		m_jumpPads[i].Update(player);
	}
}

void JumpPad_Manager::Draw()
{
	for (int i = 0; i < m_jumpPadCount; i++)
	{
        m_jumpPads[i].Draw();
	}

}