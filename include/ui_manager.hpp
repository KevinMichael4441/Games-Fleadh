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
		void updateUI(float& t_dt, Vector2 t_pos);
		void drawUI();

		StingAnim stingAnim;

	private:
		
		GameState screen;
		Vector2 center;

		int const WIDTH = 160;
		int const HEIGHT = 60;

		Vector2 button1Pos{0.0f,0.0f};
		Vector2 button2Pos{0.0f,0.0f};
		Vector2 button3Pos{0.0f,0.0f};
		Vector2 button4Pos{0.0f,0.0f};
		Vector2 button5Pos{0.0f,0.0f};

		void initialize();
		void loadUI(Vector2& t_pos);
		void unloadUI();

		void loadStartUI();
		void updateStartUI();
		void drawStartUI();
		void unloadStartUI();

		void loadMenuUI();
		void updateMenuUI();
		void drawMenuUI();
		void unloadMenuUI();

		void loadGameplayUI();
		void updateGameplayUI(float& t_dt);
		void drawGameplayUI();
		void unloadGameplayUI();

		void loadPauseUI();
		void updatePauseUI();
		void drawPauseUI();
		void unloadPauseUI();

		void loadInstructionUI();
		void updateInstructionUI();
		void drawInstructionUI();
		void unloadInstructionUI();

		void loadEndUI(Vector2& t_pos);
		void updateEndUI(float& t_dt);
		void drawEndUI();
		void unloadEndUI();
};

#endif