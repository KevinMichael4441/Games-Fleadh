#ifndef FSM_H
#define FSM_H

#include <stdio.h>
#include <raylib.h>
#include <iostream>

#include "utility/commands.h"
#include "utility/events.h"
#include "utility/states.h"

class FSM
{
    public:
        FSM();
        ~FSM();
        void update();
    private:
        Commands activeCommand;
        Commands newCommand;
        Event event;
        State state;

        // Array of states that the current state can transition into
        int arraySize = 0;
        State validIdleTransitions[2];
        State validMovingTransitions[2];
        State validJumpingTransitions[2];

        void initialize();
        void initIdleTransitions();
        void initMovingTransitions();
        void initJumpingTransitions();

        bool isCommandActive(Commands t_active);
        void pollInputs(); // If command is not active, new active command
        void newEvent();
        void newTransition();
        void checkValidTransition(State t_state);

        void enterState(State t_state);
        void updateState();
        void exitState();
};

#endif