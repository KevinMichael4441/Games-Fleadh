//=================================================================
// R36S Controller Button Tester
// NOTE: Cross-Platform Input StarterKit
// Works on Linux | Windows | Web | R36S
//=================================================================

#include "game.h"

#if defined(PLATFORM_R36S) || defined(PLATFORM_LINUX)
#if defined(GRAPHICS_API_OPENGL_ES3)
#include <GLES3/gl3.h>
#elif defined(GRAPHICS_API_OPENGL_ES2)
#include <GLES2/gl2.h>
#else
#include <GL/gl.h>
#endif
#endif

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

//=================================================================
// Mainline
//=================================================================
int main(void)
{
	Init();

#if defined(PLATFORM_WEB)
	// Web: Emscripten drives the loop, do not use while(!WindowShouldClose())
	// see https://github.com/raysan5/raylib/wiki/Working-for-Web-(HTML5)
	emscripten_set_main_loop(GameLoop, 0, 1);
#else
	// Native: standard Raylib loop
	while (!WindowShouldClose())
		GameLoop();
#endif

	Exit();
	return 0;
}