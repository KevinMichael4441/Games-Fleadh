#ifndef COLLECTIBLES_H
#define COLLECTIBLES_H

#include <raylib.h>
#include "ooze.hpp"

static const int MAX_COLLECTIBLES = 100;

//------------------Collectible------------------//

class Collectible
{
private:
    Texture m_texture;
    Vector2 m_position;
    float m_radius;
    bool m_active;
    float m_bobTimer;

public:
    Collectible();
    void Initialize(Vector2 pos, float radius);

    void Update(Ooze& player, int& score, float dt);
    void Draw() const;

    bool IsActive() const;
};

//------------------Manager------------------//

class Collectibles_Manager
{
public:
    Collectibles_Manager();
    void Initialize(Vector2 pos, float radius);

    void Update(Ooze& player, int& score, float dt);
    void Draw() const;

private:
    Collectible m_collectibles[MAX_COLLECTIBLES];
};

#endif