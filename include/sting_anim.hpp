#ifndef STING_ANIM_HPP
#define STING_ANIM_HPP

#include <raylib.h>
#include "constants.h"

class StingAnim
{

	public:
		StingAnim();
		~StingAnim();

		void setup(Vector2& t_pos);
		void update(float& t_dt);
		void play();
		void pause();
		void draw();

		bool playingAnim();
		bool timeToSpawn();

		void setStingPos(Vector2& t_pos);

		bool loadSpriteSheetA(const char* t_path);
		bool loadSpriteSheetB(const char* t_path);
		bool loadSpriteSheetC(const char* t_path);
		void unloadSpriteSheet();

		bool isPlaying;
		bool spawn;
		bool paused;

	private:

		float timer{0.0f};
		float const MAX_DURATION{3.0f};
		float const SPEED{128.0f};

		float barHeight;

		Texture2D m_sheetA{};
		Texture2D m_sheetB{};
		Texture2D m_sheetC{};
		bool m_loadedA{false};
		bool m_loadedB{false};
		bool m_loadedC{false};

		static constexpr int FRAME_W = 96;
    	static constexpr int FRAME_H = 64;
    	static constexpr int FRAME_COUNT = 13;

		int m_variant{0};  // m_sheetA = 0 and m_sheetB = 1
		int m_frame{0};

		Rectangle background;
		Rectangle highBar;
		Rectangle lowBar;

		Color fader;
		float alpha;
		float const MAX_ALPHA = 1.0f;
		float const FADE_SPEED = 0.075f;

		float m_frameTimer{0.0f};
		float const FRAME_FPS{13.0f};
		float const SCALE{2.0};
		float const ANIM_DELAY{0.5f};
};

#endif