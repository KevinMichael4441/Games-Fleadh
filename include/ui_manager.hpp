#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <raylib.h>
#include <iostream>

#include "utility/screens.hpp"

class UI_Manager
{
	public:
		UI_Manager();
		~UI_Manager();

		void UIdevToggle();

		void changeUI(Screen t_newScreen);
		void updateUI();
		void drawUI();
	private:
		
		Screen screen;

		void initialize();
		void unloadUI();
		void loadUI();

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