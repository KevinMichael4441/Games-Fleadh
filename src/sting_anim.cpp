#include "sting_anim.hpp"

StingAnim::StingAnim(){
	playing = false;
	barHeight = 100.0f;

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
	playing = false;
	barHeight = 100.0f;

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
}
void StingAnim::play(){
	if(playing == false){
		playing = true;
	}
	if(playing == true){
		if(highBar.x < background.x)
		{
			highBar.x += SPEED;
		}
		if(lowBar.x > background.x)
		{
			lowBar.x -= SPEED;
		}
	}
}
void StingAnim::draw(){
	//DrawRectangleRec(background, BLACK);
	DrawRectangleRec(highBar, ORANGE);
	DrawRectangleRec(lowBar, YELLOW);
}