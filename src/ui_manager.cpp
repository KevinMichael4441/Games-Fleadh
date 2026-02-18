#include "ui_manager.hpp"

UI_Manager::UI_Manager(){
	initialize();
	std::cout << "UI_Manager Object Created\n";
}
UI_Manager::~UI_Manager(){
	std::cout << "UI_Manager Object Destroyed\n";
}

void UI_Manager::UIdevToggle(){
	if(IsKeyPressed(KEY_ONE)){
		changeUI(GAME_START);
	}
	else if(IsKeyPressed(KEY_TWO)){
		changeUI(GAME_PLAY);
	}
	else if(IsKeyPressed(KEY_THREE)){
		changeUI(GAME_PAUSE);
	}
	else if(IsKeyPressed(KEY_FOUR)){
		changeUI(GAME_END);
	}
}

void UI_Manager::changeUI(GameState t_newScreen){
	if(t_newScreen != screen){
		unloadUI();
		screen = t_newScreen;
		loadUI();
	}
}
void UI_Manager::updateUI(){
	switch (screen){
		case GAME_START:
			updateStartUI();
		break;
		case GAME_PLAY:
			updateGameplayUI();
		break;
		case GAME_PAUSE:
			updatePauseUI();
		break;
		case GAME_END:
			updateEndUI();
		break;
	}
}
void UI_Manager::drawUI(){
	switch (screen){
		case GAME_START:
			drawStartUI();
		break;
		case GAME_PLAY:
			drawGameplayUI();
		break;
		case GAME_PAUSE:
			drawPauseUI();
		break;
		case GAME_END:
			drawEndUI();
		break;
	}
}

void UI_Manager::initialize(){
	screen = GAME_PLAY;
}
void UI_Manager::loadUI(){
	switch (screen){
		case GAME_START:
			loadStartUI();
		break;
		case GAME_PLAY:
			loadGameplayUI();
		break;
		case GAME_PAUSE:
			loadPauseUI();
		break;
		case GAME_END:
			loadEndUI();
		break;
	}
}
void UI_Manager::unloadUI(){
	switch (screen){
		case GAME_START:
			unloadStartUI();
		break;
		case GAME_PLAY:
			unloadGameplayUI();
		break;
		case GAME_PAUSE:
			unloadPauseUI();
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

}
void UI_Manager::unloadStartUI(){
	std::cout << "Unloading START Screen UI\n";
}

void UI_Manager::loadGameplayUI(){
	std::cout << "Loading GAMEPLAY Screen UI\n";
}
void UI_Manager::updateGameplayUI(){

}
void UI_Manager::drawGameplayUI(){

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

}
void UI_Manager::unloadPauseUI(){
	std::cout << "Unloading PAUSE Screen UI\n";
}

void UI_Manager::loadEndUI(){
	std::cout << "Loading END Screen UI\n";
}
void UI_Manager::updateEndUI(){

}
void UI_Manager::drawEndUI(){

}
void UI_Manager::unloadEndUI(){
	std::cout << "Unloading END Screen UI\n";
}