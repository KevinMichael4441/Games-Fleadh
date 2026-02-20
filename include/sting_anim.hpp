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
		void play(float& t_dt);
		void draw();
		bool playingAnim();

		bool isPlaying;

	private:

		float timer{0.0f};
		float const MAX_DURATION{2.0f};
		float const SPEED{128.0f};

		float barHeight;

		Rectangle background;
		Rectangle highBar;
		Rectangle lowBar;

		Color fader;
		float alpha;
		float const MAX_ALPHA = 255.0f;
		float const FADE_SPEED = 0.075f;
};

#endif