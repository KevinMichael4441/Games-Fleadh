#ifndef COLLECTIBLES_H
#define COLLECTIBLES_H

#include <raylib.h>
#include "ooze.hpp"

static const int MAX_COLLECTIBLES = 100;

class Collectible
{
private:
    Vector2 m_position;
    float m_radius;
    bool m_active;

public:
    Collectible();void Initialize(Vector2 pos, float radius);

    void Update(Ooze& player, int& score);

    void Draw() const;

    bool IsActive() const;

    Vector2 GetPosition() const;
    float GetRadius() const;
};

//------------------Manager------------------//

class Collectibles_Manager
{
private:
    Collectible m_collectibles[MAX_COLLECTIBLES];
    int m_count;

public:
    Collectibles_Manager();

    void AddCollectible(Vector2 pos, float radius);

    void Update(Ooze& player, int& score);

    void Draw() const;

    void Reset();
};


#endif