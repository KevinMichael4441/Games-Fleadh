#ifndef UI_MANAGER_HPP
#define UI_MANAGER_HPP

#include <raylib.h>
#include <iostream>

#include "command.h"
#include "gamestates.hpp"
#include "sting_anim.hpp"
#include "command.h"

typedef enum Button{
	BUTTON_START,
	BUTTON_PAUSE,
	BUTTON_INSTRUCTION,
	BUTTON_EXIT
} Button;

class UI_Manager
{
	public:
		UI_Manager();
		~UI_Manager();

		void initialize();
		void changeUI(GameState t_newScreen, Vector2 t_pos);
		GameState updateUI(float& t_dt, Vector2 t_pos, Command& t_activeCommand);
		void drawUI();

		StingAnim stingAnim;

	private:
		
		GameState screen;
		Vector2 center;
		Vector2 corner;

		Button activeSelection;
		Button newSelection;
		bool getNewCommand = true;
		float commandTimer{0.0f};
		float const MAX_DELAY{0.1f};

		int const WIDTH = 160;
		int const HEIGHT = 60;

		Vector2 selectPos{0.0f,0.0f};
		int selectWidth = 160;
		int selectHeight = 10;

		Vector2 button1Pos{0.0f,0.0f};
		Vector2 button2Pos{0.0f,0.0f};
		Vector2 button3Pos{0.0f,0.0f};

		bool paused{false};
		Color fader;
		float alpha{0.0f};
		float const MAX_ALPHA = 0.5f;
		float const FADE_SPEED = 0.075f;

		void recenter(Vector2& t_pos);

		void loadUI(Vector2& t_pos);
		void unloadUI();

		void loadStartUI();
		void updateStartUI();
		void drawStartUI();
		void unloadStartUI();

		void loadMenuUI();
		void updateMenuUI(float& t_dt, Command& t_activeCommand);
		void drawMenuUI();
		void unloadMenuUI();

		void loadGameplayUI(Vector2& t_pos);
		void updateGameplayUI(float& t_dt, Vector2& t_pos);
		void drawGameplayUI();
		void unloadGameplayUI();

		void loadPauseUI(Vector2& t_pos);
		void updatePauseUI(float& t_dt, Command& t_newCommand);
		void drawPauseUI();
		void unloadPauseUI();

		void loadInstructionUI();
		void updateInstructionUI(float& t_dt, Command& t_newCommand);
		void drawInstructionUI();
		void unloadInstructionUI();

		void loadEndUI(Vector2& t_pos);
		void updateEndUI(float& t_dt);
		void drawEndUI();
		void unloadEndUI();
};

#endif