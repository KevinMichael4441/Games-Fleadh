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
        State m_previousState;

        // Array of states that the current state can transition into
        static int const m_numIdleTransitions = 2;
        std::pair<Event, State> validIdleTransitions[m_numIdleTransitions];
       
        static int const m_numMovingTransitions = 3;
        std::pair<Event, State> validMovingTransitions[m_numMovingTransitions];

        static int const m_numJumpingTransitions = 3;
        std::pair<Event, State> validJumpingTransitions[m_numJumpingTransitions];

        static int const m_numCollideDownTransitions = 2;
        std::pair<Event, State> validCollideDownTransitions[2];

        static int const m_numCollideUpTransitions = 1;
        std::pair<Event, State> validCollideUpTransitions[m_numCollideUpTransitions];

        static int const m_numCollideHorizontalTransitions = 2;
        std::pair<Event, State> validCollideHorizontalTransitions[m_numCollideUpTransitions];
        
        
        


        void initialize();
        void initIdleTransitions();
        void initMovingTransitions();
        void initJumpingTransitions();
        void initCollideDownTransitions();
        void initCollideUpTransitions();
        void initCollideHorizontalTransitions();
        
};

#endif