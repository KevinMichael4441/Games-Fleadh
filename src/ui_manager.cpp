#include "ui_manager.hpp"

UI_Manager::UI_Manager(){
	initialize();
	std::cout << "UI_Manager Object Created\n";
}
UI_Manager::~UI_Manager(){
	std::cout << "UI_Manager Object Destroyed\n";
}

void UI_Manager::changeUI(GameState t_newScreen, Vector2 t_pos){
	if(t_newScreen != screen){
		unloadUI();
		screen = t_newScreen;
		loadUI(t_pos);
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
void UI_Manager::loadUI(Vector2& t_pos){
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
			loadEndUI(t_pos);
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

void UI_Manager::loadEndUI(Vector2& t_pos){
	std::cout << "Loading END Screen UI\n";
	stingAnim.setup(t_pos);
}
void UI_Manager::updateEndUI(){
	stingAnim.play();
}
void UI_Manager::drawEndUI(){
	stingAnim.draw();
}
void UI_Manager::unloadEndUI(){
	std::cout << "Unloading END Screen UI\n";
}