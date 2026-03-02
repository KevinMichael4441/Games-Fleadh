#include "sting_anim.hpp"

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

}

void StingAnim::setup(Vector2& t_pos){
	barHeight = 100.0f;
	fader = (Color){0,0,0,0};
	alpha = 0.0f;
	spawn = false;

	if(t_pos.x > SCREEN_WIDTH / 2 && t_pos.x < UPPER_LIMIT - SCREEN_WIDTH / 2){
		background.x = t_pos.x - (SCREEN_WIDTH / 2);
	}
	if(t_pos.y > SCREEN_HEIGHT / 2 && t_pos.y < UPPER_LIMIT - SCREEN_HEIGHT / 2){
		background.y = t_pos.y - (SCREEN_HEIGHT / 2);
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
	if(t_pos.x > SCREEN_WIDTH / 2 && t_pos.x < UPPER_LIMIT - SCREEN_WIDTH / 2){
		background.x = t_pos.x - (SCREEN_WIDTH / 2);
	}
	if(t_pos.y > SCREEN_HEIGHT / 2 && t_pos.y < UPPER_LIMIT - SCREEN_HEIGHT / 2){
		background.y = t_pos.y - (SCREEN_HEIGHT / 2);
	}

	highBar.y = background.y + (SCREEN_HEIGHT / 2) - barHeight;
	lowBar.y = background.y + (SCREEN_HEIGHT / 2);
}

void StingAnim::update(float& t_dt){
	if(isPlaying == true && paused == false){
		timer += t_dt;
		if(timer < MAX_DURATION / 2)
		{
			if(highBar.x < background.x)
			{
				highBar.x += SPEED;
			}
			if(lowBar.x > background.x)
			{
				lowBar.x -= SPEED;
			}
			if(alpha < MAX_ALPHA)
			{
				alpha += FADE_SPEED;
				fader = Fade(fader, alpha);
			}
		}
		else if(timer > MAX_DURATION / 2 && timer < MAX_DURATION)
		{
			if(!spawn){spawn = true;}
			if(highBar.x >= background.x - SCREEN_WIDTH)
			{
				highBar.x -= SPEED;
			}
			if(lowBar.x <= background.x + SCREEN_WIDTH)
			{
				lowBar.x += SPEED;
			}
			if(alpha > 0.0f)
			{
				alpha -= FADE_SPEED;
				fader = Fade(fader, alpha);
			}
			if(alpha <= 0.0f)
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
}