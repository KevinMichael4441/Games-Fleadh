#include "sting_anim.hpp"
#include <cstdio>

StingAnim::StingAnim(){
	isPlaying = false;
	spawn = false;
	paused = false;
	barHeight = 100.0f;
	fader = (Color){0,0,0,0};
	alpha = 0.0f;

	background.x = 0.0f;
	background.y = 0.0f;
	background.width = SCREEN_WIDTH;
	background.height = SCREEN_HEIGHT;

	highBar.x = 0.0f;
	highBar.y = (SCREEN_HEIGHT / 2) - barHeight;
	highBar.width = SCREEN_WIDTH;
	highBar.height = barHeight;

	lowBar.x = SCREEN_WIDTH;
	lowBar.y = SCREEN_HEIGHT / 2;
	lowBar.width = SCREEN_WIDTH;
	lowBar.height = barHeight;
}
StingAnim::~StingAnim(){
	unloadSpriteSheet();
}

void StingAnim::setup(Vector2& t_pos){
	timer = 0.0f;
	barHeight = 100.0f;
	fader = (Color){0,0,0,0};
	alpha = 0.0f;
	spawn = false;
	m_frame = 0;
	m_frameTimer = 0.0f;
	m_variant = GetRandomValue(0, 2);

	if(t_pos.x > SCREEN_WIDTH / 2){
		background.x = t_pos.x - (SCREEN_WIDTH / 2);
	}
	else{
		background.x = 0.0f;
	}
	if(t_pos.y > SCREEN_HEIGHT / 2){
		background.y = t_pos.y - (SCREEN_HEIGHT / 2);
	}
	else{
		background.y = 0.0f;
	}
	
	background.width = SCREEN_WIDTH;
	background.height = SCREEN_HEIGHT;

	highBar.x = background.x - SCREEN_WIDTH;
	highBar.y = background.y + (SCREEN_HEIGHT / 2) - barHeight;
	highBar.width = SCREEN_WIDTH;
	highBar.height = barHeight;

	lowBar.x = background.x + SCREEN_WIDTH;
	lowBar.y = background.y + (SCREEN_HEIGHT / 2);
	lowBar.width = SCREEN_WIDTH + 500;
	lowBar.height = barHeight;

	if(isPlaying == false){
		isPlaying = true;
	}
}

void StingAnim::setStingPos(Vector2& t_pos)
{
	if(t_pos.x > SCREEN_WIDTH / 2){
		background.x = t_pos.x - (SCREEN_WIDTH / 2);
	}
	else{
		background.x = 0.0f;
	}
	if(t_pos.y > SCREEN_HEIGHT / 2){
		background.y = t_pos.y - (SCREEN_HEIGHT / 2);
	}
	else{
		background.y = 0.0f;
	}

	highBar.y = background.y + (SCREEN_HEIGHT / 2) - barHeight;
	lowBar.y = background.y + (SCREEN_HEIGHT / 2);
}

void StingAnim::update(float& t_dt)
{
    if (isPlaying == true && paused == false)
    {
        timer += t_dt;

        bool anyLoaded = (m_loadedA && m_sheetA.id != 0) || (m_loadedB && m_sheetB.id != 0);

		if (anyLoaded)
		{
    		if (timer >= ANIM_DELAY)
    		{
        		m_frameTimer += t_dt;
        		float frameTime = 1.0f / FRAME_FPS;

        		while (m_frameTimer >= frameTime)
        		{
            		m_frameTimer -= frameTime;

            		if (m_frame < FRAME_COUNT - 1)
            		{
                		m_frame++;
            		}
            		else
            		{
                		break;
            		}
				}
        	}
   		}
	


        if (timer < MAX_DURATION / 2)
        {
            if (highBar.x < background.x)
            {
                highBar.x += SPEED;
            }
            if (lowBar.x > background.x)
            {
                lowBar.x -= SPEED;
            }
            if (alpha < MAX_ALPHA)
            {
                alpha += FADE_SPEED;
                fader = Fade(fader, alpha);
            }
        }
        else if (timer > MAX_DURATION / 2 && timer < MAX_DURATION)
        {
            if (!spawn)
            {
                spawn = true;
            }

            if (highBar.x >= background.x - SCREEN_WIDTH)
            {
                highBar.x -= SPEED;
            }
            if (lowBar.x <= background.x + SCREEN_WIDTH)
            {
                lowBar.x += SPEED;
            }
            if (alpha > 0.0f)
            {
                alpha -= FADE_SPEED;
                fader = Fade(fader, alpha);
            }
            if (alpha <= 0.0f)
            {
                isPlaying = false;
                timer = 0.0f;
            }
        }
    }
}

void StingAnim::play(){
	if(paused)
	{
		paused = false;
	}
}
void StingAnim::pause(){
	if(!paused)
	{
		paused = true;
	}
}

bool StingAnim::playingAnim()
{
	return isPlaying;
}
bool StingAnim::timeToSpawn()
{
	return spawn;
}
void StingAnim::draw(){
	DrawRectangleRec(background, fader);
	DrawRectangleRec(highBar, ORANGE);
	DrawRectangleRec(lowBar, ORANGE);
	Texture2D tex;
	bool loaded;

	if (m_variant == 0)
	{
    	tex = m_sheetA;
    	loaded = m_loadedA;
	}
	else if (m_variant == 1)
	{
    	tex = m_sheetB;
    	loaded = m_loadedB;
	}
	else
	{
		tex = m_sheetC;
		loaded = m_loadedC;
	}

	    if (loaded && tex.id != 0 && timer >= ANIM_DELAY)
    	{
        	Rectangle src;
        	src.x = (float)(m_frame * FRAME_W);
        	src.y = 0.0f;
        	src.width = (float)FRAME_W;
        	src.height = (float)FRAME_H;

        	float centerX = background.x + (SCREEN_WIDTH  * 0.5f);
    		float centerY = background.y + (SCREEN_HEIGHT * 0.5f);

        	Rectangle dst;
        	dst.width  = (float)FRAME_W * SCALE;
        	dst.height = (float)FRAME_H * SCALE;
        	dst.x = centerX - dst.width * 0.5f;
        	dst.y = centerY - dst.height * 0.5f;

        	DrawTexturePro(tex, src, dst, Vector2{0,0}, 0.0f, WHITE);
    	}
}

bool StingAnim::loadSpriteSheetA(const char* t_path)
{
    if (m_loadedA)
    {
        return true;
    }

    m_sheetA = LoadTexture(t_path);

    if (m_sheetA.id == 0)
    {
        m_loadedA = false;
        return false;
    }

    m_loadedA = true;
    return true;
}

bool StingAnim::loadSpriteSheetB(const char* t_path)
{
    if (m_loadedB)
    {
        return true;
    }

    m_sheetB = LoadTexture(t_path);

    if (m_sheetB.id == 0)
    {
        m_loadedB = false;
        return false;
    }

    m_loadedB = true;
    return true;
}

bool StingAnim::loadSpriteSheetC(const char* t_path)
{
    if (m_loadedC)
    {
        return true;
    }

    m_sheetC = LoadTexture(t_path);

    if (m_sheetC.id == 0)
    {
        m_loadedC = false;
        return false;
    }

    m_loadedC = true;
    return true;
}

void StingAnim::unloadSpriteSheet()
{
    if (m_loadedA)
    {
        UnloadTexture(m_sheetA);
        m_sheetA = Texture2D{};
        m_loadedA = false;
    }

    if (m_loadedB)
    {
        UnloadTexture(m_sheetB);
        m_sheetB = Texture2D{};
        m_loadedB = false;
    }

	if (m_loadedC)
    {
        UnloadTexture(m_sheetC);
        m_loadedC = false;
    }
}