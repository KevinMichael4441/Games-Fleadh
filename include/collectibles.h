#ifndef COLLECTIBLES_H
#define COLLECTIBLES_H

#include <raylib.h>
#include "ooze.hpp"
#include "level_loader.h"

static const int MAX_COLLECTIBLES = 30;

//------------------Collectible------------------//

class Collectible
{
private:
    static Texture2D m_texture;
    static bool m_textureLoaded;
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
    void Initialize(LevelData* level, float radius);

    void Update(Ooze& player, int& score, float dt);
    void Draw() const;

private:
    Collectible m_collectibles[MAX_COLLECTIBLES];
    int m_collectibleCount = 0;
};

#endif