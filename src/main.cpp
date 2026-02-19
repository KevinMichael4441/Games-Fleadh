#include <stdio.h>
#include <cstring>

#include "game.hpp"
#include "constants.h"
#include "time.h"

// Main Entry Point
int main(void)
{
	srand(time(nullptr));

	const char *qemu_headless = getenv("QEMU_HEADLESS");
	if (qemu_headless && strcmp(qemu_headless, "1") == 0)
	{
		SetTraceLogLevel(LOG_INFO);
		TraceLog(LOG_INFO,"QEMU Headless");
		return 0; // No Graphics
	}

	Game game;
	game.Run();

	return 1; // success
}