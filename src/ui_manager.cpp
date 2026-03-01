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
	activeSelection = BUTTON_START;
	newSelection = BUTTON_START;
}

void UI_Manager::changeUI(GameState t_newScreen, Vector2 t_pos){
	if(t_newScreen != screen){
		center = {t_pos.x - SCREEN_WIDTH / 2, t_pos.y - SCREEN_HEIGHT /2};
		unloadUI();
		screen = t_newScreen;
		loadUI(t_pos);
	}
}
GameState UI_Manager::updateUI(float& t_dt, Vector2 t_pos, Command& t_activeCommand){

	if(t_pos.x >= SCREEN_WIDTH / 2)
	{
		center.x = t_pos.x - SCREEN_WIDTH / 2;
	}
	else
	{
		center.x = SCREEN_WIDTH / 2;
	}
	if(t_pos.y >= SCREEN_HEIGHT / 2)
	{
		center.y = t_pos.y - SCREEN_HEIGHT / 2;
	}
	else
	{
		center.y = SCREEN_HEIGHT / 2;
	}
	
	switch (screen){
		case GAME_START:
			updateStartUI();
		break;
		case GAME_MENU:
			updateMenuUI(t_dt, t_activeCommand);
		break;
		case GAME_PLAY:
			updateGameplayUI(t_dt);
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
	return screen;
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
	button2Pos = {center.x + 400, center.y + 100};
	button3Pos = {center.x + 400, center.y + 210};
	button4Pos = {center.x + 400, center.y + 320};

	newSelection = BUTTON_START;
	activeSelection = BUTTON_START;
	selectPos = {button1Pos.x - selectWidth / 2, button1Pos.y + 35};
}
void UI_Manager::updateMenuUI(float& t_dt, Command& t_newCommand){
	if(getNewCommand)
	{
		switch (activeSelection)
		{
			case BUTTON_START:
				if(t_newCommand == ATTACK_PRIMARY || t_newCommand == START_GAME || t_newCommand == ACTION_JUMP){screen = GAME_PLAY;}
				else if(t_newCommand == MOVE_UP){newSelection = BUTTON_INSTRUCTION;}
				else if(t_newCommand == MOVE_DOWN){newSelection = BUTTON_EXIT;}
				else if(t_newCommand == MOVE_RIGHT){newSelection = BUTTON_ACHIEVMENT;}
			break;
			case BUTTON_PAUSE:
			break;
			case BUTTON_INSTRUCTION:
				if(t_newCommand == ATTACK_PRIMARY || t_newCommand == START_GAME || t_newCommand == ACTION_JUMP){screen = GAME_INSTRUCTION;}
				else if(t_newCommand == MOVE_UP){newSelection = BUTTON_EXIT;}
				else if(t_newCommand == MOVE_DOWN){newSelection = BUTTON_ACHIEVMENT;}
				else if(t_newCommand == MOVE_LEFT){newSelection = BUTTON_START;}
			break;
			case BUTTON_ACHIEVMENT:
				if(t_newCommand == ATTACK_PRIMARY || t_newCommand == START_GAME || t_newCommand == ACTION_JUMP){}
				else if(t_newCommand == MOVE_UP){newSelection = BUTTON_INSTRUCTION;}
				else if(t_newCommand == MOVE_DOWN){newSelection = BUTTON_EXIT;}
				else if(t_newCommand == MOVE_LEFT){newSelection = BUTTON_START;}
			break;
			case BUTTON_EXIT:
				if(t_newCommand == ATTACK_PRIMARY || t_newCommand == START_GAME || t_newCommand == ACTION_JUMP){}
				else if(t_newCommand == MOVE_UP){newSelection = BUTTON_ACHIEVMENT;}
				else if(t_newCommand == MOVE_DOWN){newSelection = BUTTON_INSTRUCTION;}
				else if(t_newCommand == MOVE_LEFT){newSelection = BUTTON_START;}
			break;
		}
	

		if(newSelection != activeSelection)
		{
			activeSelection = newSelection;
			getNewCommand = false;

			switch (activeSelection)
			{
				case BUTTON_START:
					selectWidth = 160;
					selectHeight = 5;
					selectPos = {button1Pos.x - (selectWidth / 2), button1Pos.y + 25};
				break;
				case BUTTON_INSTRUCTION:
					selectWidth = 120;
					selectHeight = 5;
					selectPos = {button2Pos.x + (WIDTH / 2) - (selectWidth / 2), button2Pos.y + HEIGHT - 15};
				break;
				case BUTTON_ACHIEVMENT:
					selectWidth = 120;
					selectHeight = 5;
					selectPos = {button3Pos.x + (WIDTH / 2) - (selectWidth / 2), button3Pos.y + HEIGHT - 15};
				break;
				case BUTTON_EXIT:
					selectWidth = 120;
					selectHeight = 5;
					selectPos = {button4Pos.x + (WIDTH / 2) - (selectWidth / 2), button4Pos.y + HEIGHT - 15};
				break;
			}
		}
	}
	else {commandTimer += t_dt;}

	if(commandTimer >= MAX_DELAY)
	{
		getNewCommand = true;
		commandTimer = 0.0f;
	}
}
void UI_Manager::drawMenuUI(){

	DrawRectangle(0,0,SCREEN_WIDTH,SCREEN_HEIGHT, DARKGRAY);

	DrawCircle(button1Pos.x, button1Pos.y + 15,  125.0f, DARKPURPLE);
	DrawCircle(button1Pos.x, button1Pos.y,  125.0f, RED);
	DrawRectangle(button2Pos.x, button2Pos.y + 10,WIDTH,HEIGHT, DARKGREEN);
	DrawRectangle(button2Pos.x, button2Pos.y,WIDTH,HEIGHT, GREEN);
	DrawRectangle(button3Pos.x, button3Pos.y + 10,WIDTH,HEIGHT, DARKGREEN);
	DrawRectangle(button3Pos.x, button3Pos.y,WIDTH,HEIGHT, GREEN);
	DrawRectangle(button4Pos.x, button4Pos.y + 10,WIDTH,HEIGHT, DARKGREEN);
	DrawRectangle(button4Pos.x, button4Pos.y,WIDTH,HEIGHT, GREEN);

	DrawRectangle(selectPos.x, selectPos.y, selectWidth, selectHeight, WHITE);

	DrawText(TextFormat("START"), button1Pos.x - 67.5, button1Pos.y - 15, 40, WHITE);
	DrawText(TextFormat("INSTRUCTIONS"), button2Pos.x + 25, button2Pos.y + 30 - 7.5f, 15, WHITE);
	DrawText(TextFormat("ACHIEVMENTS"), button3Pos.x + 25, button3Pos.y + 30 - 7.5f, 15, WHITE);
	DrawText(TextFormat("EXIT"), button4Pos.x + 25, button4Pos.y + 30 - 7.5f, 15, WHITE);
}
void UI_Manager::unloadMenuUI(){
	std::cout << "Unloading MENU Screen UI\n";
}

void UI_Manager::loadGameplayUI(){
	std::cout << "Loading GAMEPLAY Screen UI\n";
	if(stingAnim.playingAnim()){stingAnim.play();}
}
void UI_Manager::updateGameplayUI(float& t_dt){
	if(stingAnim.playingAnim()){ stingAnim.update(t_dt);}
}
void UI_Manager::drawGameplayUI(){
}
void UI_Manager::unloadGameplayUI(){
	std::cout << "Unloading GAMEPLAY Screen UI\n";
}

void UI_Manager::loadPauseUI(){
	std::cout << "Loading PAUSE Screen UI\n";
	if(stingAnim.playingAnim()){stingAnim.pause();}
}
void UI_Manager::updatePauseUI(){

}
void UI_Manager::drawPauseUI(){
	if(stingAnim.playingAnim()){stingAnim.draw();}
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
	if(stingAnim.playingAnim()){stingAnim.play();}
	else if(!stingAnim.playingAnim()){stingAnim.setup(t_pos);}
}
void UI_Manager::updateEndUI(float& t_dt){
	if(stingAnim.playingAnim()){ stingAnim.update(t_dt);}
}
void UI_Manager::drawEndUI(){
	stingAnim.draw();
}
void UI_Manager::unloadEndUI(){
	std::cout << "Unloading END Screen UI\n";
}