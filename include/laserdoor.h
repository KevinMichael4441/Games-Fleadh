#ifndef LASERDOOR_H
#define LASERDOOR_H

#include <raylib.h>
#include <raymath.h>

#include "constants.h"

#include "ooze.hpp"

extern "C" {
#include "cute_c2.h"
}

//------------------Key------------------//

class Key
{
public:
    Key();
	void Initialize(Vector2 pos, float radius);

    bool Update(Ooze& player, float dt);
    void Draw() const;

private:
	Texture2D m_texture;
	bool m_active;
	float m_bobTimer;
    Vector2 m_position;
    float m_radius;
};

//------------------LaserDoor------------------//

class LaserDoor
{
public:
	LaserDoor();
	void Initialize(Vector2 pos);

	void Update(Ooze &player, float dt);
	void Disactivate();
	void Draw() const;

private:
	static const int MAX_BARS = 6;
	static const int m_BAR_WIDTH = 4;
	static const int m_BAR_HEIGHT = 128;
	static const int m_WIDTH = 64;
	static const int m_HEIGHT = 128;

	bool m_active;
	c2AABB m_boundingBox;
	Rectangle m_bars[MAX_BARS];

	Texture2D m_texture;
	int m_currentFrame;
	int m_targetFrame;
	bool m_animating;
	float m_animTimer;
	float m_animSpeed;
};

//------------------Manager------------------//

class LaserDoor_Manager
{
	public:
	LaserDoor_Manager();
	void Initialize(Vector2 lazer_pos, Vector2 key_pos, float key_radius);

	void Update(Ooze& player, float dt);
	void Draw() const;

	private:
	std::pair<Key, LaserDoor> m_laserdoor_pairs;
};

#endif //laserdoor.h