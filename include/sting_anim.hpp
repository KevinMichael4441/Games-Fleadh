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
		void play();
		void draw();

	private:
		bool playing;

		float const SPEED = 128.0f;

		float barHeight;

		Rectangle background;
		Rectangle highBar;
		Rectangle lowBar;
};

#endif