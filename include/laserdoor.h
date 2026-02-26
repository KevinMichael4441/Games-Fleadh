#ifndef LASERDOOR_H
#define LASERDOOR_H

#include <raylib.h>
#include <raymath.h>

#include "ooze.hpp"

#include "constants.h"

extern "C" {
#include "cute_c2.h"
}

static const int MAX_PAIRS = 10;

class LaserDoor_Manager
{
	public:
	LaserDoor_Manager();
	void Initialize(Vector2 key_pos, float key_radius, float lazer_x, float lazer_y);

	void update(Ooze& player);
	void Draw() const;

	private:
	std::pair<Key, Door>[MAX_PAIRS] laserdoorPairs;
}

class Key
{
public:
    Key();
	void Initialize(Vector2 pos, float radius);

    void Update(Ooze& player);
    void Draw() const;

    Vector2 GetPosition() const;

	private:
	bool m_active;
    Vector2 m_position;
    float m_radius;
};

class LaserDoor
{
public:
	LaserDoor();
	void initialize(float t_x, float t_y);

	void update(Ooze &t_ooze, float t_dt);
	void updateCollision(Ooze &t_ooze);
	void draw() const;

private:
	static const int MAX_BARS = 6;
	static const int m_BAR_WIDTH = 4;
	static const int m_BAR_HEIGHT = 128;
	static const int m_WIDTH = 64;
	static const int m_HEIGHT = 128;

	bool m_isActive;
	c2AABB m_boundingBox;
	Rectangle m_bars[MAX_BARS];
};

#endif //laserdoor.h