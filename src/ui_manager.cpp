#include "ui_manager.hpp"

UI_Manager::UI_Manager(){
	initialize();
	std::cout << "UI_Manager Object Created\n";
}

UI_Manager::~UI_Manager(){
	std::cout << "UI_Manager Object Destroyed\n";
}

void UI_Manager::initialize(){
	screen = GAMEPLAY;
}

void UI_Manager::changeUI(Screen t_newScreen){
	if(t_newScreen != screen){
		unloadUI();
		screen = t_newScreen;
		loadUI();
	}
}

void UI_Manager::loadUI(){
	switch (screen){
		case START:
			loadStartUI();
		break;
		case GAMEPLAY:
			loadGameplayUI();
		break;
		case PAUSE:
			loadPauseUI();
		break;
		case END:
			loadEndUI();
		break;
	}
}

void UI_Manager::updateUI(){
	switch (screen){
		case START:
			updateStartUI();
		break;
		case GAMEPLAY:
			updateGameplayUI();
		break;
		case PAUSE:
			updatePauseUI();
		break;
		case END:
			updateEndUI();
		break;
	}
}

void UI_Manager::drawUI(){
	switch (screen){
		case START:
			drawStartUI();
		break;
		case GAMEPLAY:
			drawGameplayUI();
		break;
		case PAUSE:
			drawPauseUI();
		break;
		case END:
			drawEndUI();
		break;
	}
}

void UI_Manager::unloadUI(){
	switch (screen){
		case START:
			unloadStartUI();
		break;
		case GAMEPLAY:
			unloadGameplayUI();
		break;
		case PAUSE:
			unloadPauseUI();
		break;
		case END:
			unloadEndUI();
		break;
	}
}

void UI_Manager::UIdevToggle(){
	if(IsKeyPressed(KEY_ONE)){
		changeUI(START);
	}
	else if(IsKeyPressed(KEY_TWO)){
		changeUI(GAMEPLAY);
	}
	else if(IsKeyPressed(KEY_THREE)){
		changeUI(PAUSE);
	}
	else if(IsKeyPressed(KEY_FOUR)){
		changeUI(END);
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