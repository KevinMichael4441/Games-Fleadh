#include <stdbool.h>

#include "command.h"

bool IsCommandActive(Command command, Command filter)
{
	return (command & filter) != 0;
}

void GetCommandBits(Command command, char *buffer)
{
	for (int i = COMMAND_COUNT - 1; i >= 0; i--)
	{
		buffer[COMMAND_COUNT - 1 - i] = (command & (1u << i)) ? '1' : '0';
	}
	buffer[COMMAND_COUNT] = '\0'; // null terminator
}
