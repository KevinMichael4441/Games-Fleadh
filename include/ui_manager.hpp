#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <raylib.h>
#include <iostream>

#include "gamestates.hpp"

class UI_Manager
{
	public:
		UI_Manager();
		~UI_Manager();

		void UIdevToggle(); // TEMPORARY : Switch between the different UI interfaces

		void changeUI(GameState t_newScreen);
		void updateUI();
		void drawUI();
		
	private:
		
		GameState screen;

		void initialize();
		void loadUI();
		void unloadUI();

		void loadStartUI();
		void updateStartUI();
		void drawStartUI();
		void unloadStartUI();

		void loadGameplayUI();
		void updateGameplayUI();
		void drawGameplayUI();
		void unloadGameplayUI();

		void loadPauseUI();
		void updatePauseUI();
		void drawPauseUI();
		void unloadPauseUI();

		void loadEndUI();
		void updateEndUI();
		void drawEndUI();
		void unloadEndUI();
};

#endif