#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <raylib.h>
#include <iostream>

#include "gamestates.hpp"
#include "sting_anim.hpp"

class UI_Manager
{
	public:
		UI_Manager();
		~UI_Manager();

		void changeUI(GameState t_newScreen, Vector2 t_pos);
		void updateUI(float& t_dt);
		void drawUI();

		StingAnim stingAnim;

	private:
		
		GameState screen;

		void initialize();
		void loadUI(Vector2& t_pos);
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

		void loadEndUI(Vector2& t_pos);
		void updateEndUI(float& t_dt);
		void drawEndUI();
		void unloadEndUI();
};

#endif