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

        
        bool CheckValidTransition(State t_currentState, Event t_event);
        State getState();

        private:

        State m_currentState;

        // Array of states that the current state can transition into
        std::pair<Event, State> validIdleTransitions[2];
        int const m_numIdleTransitions = 2;

        std::pair<Event, State> validMovingTransitions[2];
        int const m_numMovingTransitions = 2;

        std::pair<Event, State> validJumpingTransitions[2];
        int const m_numJumpingTransitions = 2;


        void initialize();
        void initIdleTransitions();
        void initMovingTransitions();
        void initJumpingTransitions();
        
};

#endif