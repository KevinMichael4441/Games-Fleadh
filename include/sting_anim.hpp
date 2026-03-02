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

		bool isPlaying;
		bool spawn;
		bool paused;

	private:

		float timer{0.0f};
		float const MAX_DURATION{3.0f};
		float const SPEED{128.0f};

		float barHeight;

		Rectangle background;
		Rectangle highBar;
		Rectangle lowBar;

		Color fader;
		float alpha;
		float const MAX_ALPHA = 1.0f;
		float const FADE_SPEED = 0.075f;

		float const UPPER_LIMIT = 5760.0f;
		int const FRAME_WIDTH = 46;
		int const FRAME_HEIGHT = 46;

		Vector2 position{0.0f,0.0f};
		Rectangle frame{0.0f,0.0f, FRAME_WIDTH, FRAME_HEIGHT};

		int frameCounter;
		int frameSpeed; // Spritesheet per second

};

#endif