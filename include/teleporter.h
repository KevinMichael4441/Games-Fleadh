#ifndef TELEPORTER_H
#define TELEPORTER_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "ooze.hpp"

static const int MAX_TELEPORTER_PAIRS = 10;

//------------------Teleporter------------------//

class Teleporter
{
public:
	Teleporter();
	void Initialize(Vector2 pos);

	bool Update(Ooze &player);
	void Teleport_To(Ooze &player);
	void Draw() const;

private:
	Rectangle m_boundingBox;
	static const int m_WIDTH = 32;
	static const int m_HEIGHT = 32;
};

//------------------Manager------------------//

class Teleporter_Manager
{
	public:
	Teleporter_Manager();
	void Initialize(Vector2 pos_one, Vector2 pos_two);

	void Update(Ooze& player, float dt);
	void Draw() const;

	private:
	std::pair<Teleporter, Teleporter> m_teleporter_pairs[MAX_TELEPORTER_PAIRS];
	bool m_active;
	float m_timer;
};

#endif //teleporter.h