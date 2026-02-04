#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include "command.h"

void InitInputManager();
Command PollInput();
void ExitInputManager();

#endif // INPUT_MANAGER_H