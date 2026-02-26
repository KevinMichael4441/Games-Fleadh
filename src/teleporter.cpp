#include "teleporter.h"

//------------------Manager------------------//

Teleporter_Manager::Teleporter_Manager()
{
	m_active = true;
	m_timer = 5.0f;
}

void Teleporter_Manager::Initialize(Vector2 pos_one, Vector2 pos_two)
{
	for(int i = 0; i < MAX_TELEPORTER_PAIRS; i++)
	{
		m_teleporter_pairs[i].first.Initialize(pos_one);
		m_teleporter_pairs[i].second.Initialize(pos_two);
	}
}

void Teleporter_Manager::Update(Ooze& player, float dt)
{
	if(m_active)
	{
		for(int i = 0; i < MAX_TELEPORTER_PAIRS; i++)
		{
			if(m_teleporter_pairs[i].first.Update(player))
			{
				m_active = false;
				m_teleporter_pairs[i].second.Teleport_To(player);
				break;
			}
			
			if(m_teleporter_pairs[i].second.Update(player))
			{
				m_active = false;
				m_teleporter_pairs[i].first.Teleport_To(player);
				break;
			}
		}
	}
	else
	{
		m_timer = m_timer - dt;

		if(m_timer <= 0)
		{
			m_timer = 5.0f;
			m_active = true;
		}
	}
}
void Teleporter_Manager::Draw() const
{
	for(int i = 0; i < MAX_TELEPORTER_PAIRS; i++)
	{
		m_teleporter_pairs[i].first.Draw();
		m_teleporter_pairs[i].second.Draw();
	}
}

//------------------Teleporter------------------//

Teleporter::Teleporter()
{
	m_boundingBox = {0, 0, m_WIDTH, m_HEIGHT};
}

void Teleporter::Initialize(Vector2 pos)
{
	m_boundingBox = {pos.x, pos.y, m_WIDTH, m_HEIGHT};
}

bool Teleporter::Update(Ooze &player)
{
	if (CheckCollisionPointRec(player.CalculateCenter(), m_boundingBox))
    {
		Point *currentPoint;
		for (int index = 0; index < MAX_POINTS; index++)
		{
			return true;
		}
    }

	return false;
}

void Teleporter::Teleport_To(Ooze &player)
{
	player.Reset({m_boundingBox.x + (m_WIDTH / 2), m_boundingBox.y + (m_HEIGHT / 2)});
}

void Teleporter::Draw() const
{
	DrawRectangle(m_boundingBox.x, m_boundingBox.y, m_WIDTH, m_HEIGHT, ORANGE );
}