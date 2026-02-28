#include "ui_manager.hpp"

UI_Manager::UI_Manager(){
	initialize();
	std::cout << "UI_Manager Object Created\n";
}
UI_Manager::~UI_Manager(){
	std::cout << "UI_Manager Object Destroyed\n";
}

void UI_Manager::initialize(){
	screen = GAME_PLAY;
	center = {0.0f,0.0f};
}

void UI_Manager::changeUI(GameState t_newScreen, Vector2 t_pos){
	if(t_newScreen != screen){
		center = {t_pos.x - SCREEN_WIDTH / 2, t_pos.y - SCREEN_HEIGHT /2};
		unloadUI();
		screen = t_newScreen;
		loadUI(t_pos);
	}
}
void UI_Manager::updateUI(float& t_dt, Vector2 t_pos){

	center = {t_pos.x - SCREEN_WIDTH / 2, t_pos.y - SCREEN_HEIGHT /2};
	switch (screen){
		case GAME_START:
			updateStartUI();
		break;
		case GAME_MENU:
			updateMenuUI();
		break;
		case GAME_PLAY:
			updateGameplayUI();
		break;
		case GAME_PAUSE:
			updatePauseUI();
		break;
		case GAME_INSTRUCTION:
			updateInstructionUI();
		break;
		case GAME_END:
			updateEndUI(t_dt);
		break;
	}
}
void UI_Manager::drawUI(){
	switch (screen){
		case GAME_START:
			drawStartUI();
		break;
		case GAME_MENU:
			drawMenuUI();
		break;
		case GAME_PLAY:
			drawGameplayUI();
		break;
		case GAME_PAUSE:
			drawPauseUI();
		break;
		case GAME_INSTRUCTION:
			drawInstructionUI();
		break;
		case GAME_END:
			drawEndUI();
		break;
	}
}

void UI_Manager::loadUI(Vector2& t_pos){
	switch (screen){
		case GAME_START:
			loadStartUI();
		break;
		case GAME_MENU:
			loadMenuUI();
		break;
		case GAME_PLAY:
			loadGameplayUI();
		break;
		case GAME_PAUSE:
			loadPauseUI();
		break;
		case GAME_INSTRUCTION:
			loadInstructionUI();
		break;
		case GAME_END:
			loadEndUI(t_pos);
		break;
	}
}
void UI_Manager::unloadUI(){
	switch (screen){
		case GAME_START:
			unloadStartUI();
		break;
		case GAME_MENU:
			unloadMenuUI();
		break;
		case GAME_PLAY:
			unloadGameplayUI();
		break;
		case GAME_PAUSE:
			unloadPauseUI();
		break;
		case GAME_INSTRUCTION:
			unloadInstructionUI();
		break;
		case GAME_END:
			unloadEndUI();
		break;
	}
}

void UI_Manager::loadStartUI(){
	std::cout << "Loading START Screen UI\n";
}
void UI_Manager::updateStartUI(){

}
void UI_Manager::drawStartUI(){
	DrawRectangle(center.x,center.y,SCREEN_WIDTH,SCREEN_HEIGHT, BLUE);
	DrawText(TextFormat("Start"), center.x, center.y, 30, WHITE);
}
void UI_Manager::unloadStartUI(){
	std::cout << "Unloading START Screen UI\n";
}

void UI_Manager::loadMenuUI(){
	std::cout << "Loading MENU Screen UI\n";

	button1Pos = {center.x + 200, center.y + 240}; // BigRed
	button2Pos = {center.x + 400, center.y + 90};
	button3Pos = {center.x + 400, center.y + 170};
	button4Pos = {center.x + 400, center.y + 250};
	button5Pos = {center.x + 400, center.y + 330};

}
void UI_Manager::updateMenuUI(){

}
void UI_Manager::drawMenuUI(){
	//DrawRectangle(center.x,center.y,SCREEN_WIDTH,SCREEN_HEIGHT, PURPLE);

	DrawCircle(button1Pos.x, button1Pos.y + 25,  125.0f, DARKPURPLE);
	DrawCircle(button1Pos.x, button1Pos.y,  125.0f, RED);
	DrawRectangle(button2Pos.x, button2Pos.y,WIDTH,HEIGHT, GREEN);
	DrawRectangle(button3Pos.x, button3Pos.y,WIDTH,HEIGHT, GREEN);
	DrawRectangle(button4Pos.x, button4Pos.y,WIDTH,HEIGHT, GREEN);
	DrawRectangle(button5Pos.x, button5Pos.y,WIDTH,HEIGHT, GREEN);

	DrawText(TextFormat("START"), button1Pos.x - 67.5, button1Pos.y - 7.5, 40, WHITE);
	DrawText(TextFormat("HOW TO PLAY"), button2Pos.x, button2Pos.y + 30 - 7.5f, 15, WHITE);
	DrawText(TextFormat("ACHIEVMENTS"), button3Pos.x, button3Pos.y + 30 - 7.5f, 15, WHITE);
	DrawText(TextFormat("EXIT"), button4Pos.x, button4Pos.y + 30 - 7.5f, 15, WHITE);
	DrawText(TextFormat("ELSE"), button5Pos.x, button5Pos.y + 30 - 7.5f, 15, WHITE);
}
void UI_Manager::unloadMenuUI(){
	std::cout << "Unloading MENU Screen UI\n";
}

void UI_Manager::loadGameplayUI(){
	std::cout << "Loading GAMEPLAY Screen UI\n";
}
void UI_Manager::updateGameplayUI(){

}
void UI_Manager::drawGameplayUI(){
	// DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, BLUE);
}
void UI_Manager::unloadGameplayUI(){
	std::cout << "Unloading GAMEPLAY Screen UI\n";
}

void UI_Manager::loadPauseUI(){
	std::cout << "Loading PAUSE Screen UI\n";
}
void UI_Manager::updatePauseUI(){

}
void UI_Manager::drawPauseUI(){
	//DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, BLUE);
}
void UI_Manager::unloadPauseUI(){
	std::cout << "Unloading PAUSE Screen UI\n";
}

void UI_Manager::loadInstructionUI(){
	std::cout << "Loading INSTRUCTION Screen UI\n";
}
void UI_Manager::updateInstructionUI(){

}
void UI_Manager::drawInstructionUI(){
	DrawRectangle(center.x,center.y,SCREEN_WIDTH,SCREEN_HEIGHT, BLUE);
	DrawText(TextFormat("Instructions"), center.x, center.y, 30, WHITE);
}
void UI_Manager::unloadInstructionUI(){
	std::cout << "Unloading INSTRUCTION Screen UI\n";
}

void UI_Manager::loadEndUI(Vector2& t_pos){
	std::cout << "Loading END Screen UI\n";
	stingAnim.setup(t_pos);
}
void UI_Manager::updateEndUI(float& t_dt){
	if(stingAnim.playingAnim())
	{
		stingAnim.play(t_dt);
	}
}
void UI_Manager::drawEndUI(){
	stingAnim.draw();
}
void UI_Manager::unloadEndUI(){
	std::cout << "Unloading END Screen UI\n";
}