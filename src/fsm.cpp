#include "fsm.h"

FSM::FSM(){
    std::cout << "FSM Object Created\n";
    initialize();
}

FSM::~FSM(){
    std::cout << "FSM Object Destroyed\n";
}

void FSM::initialize(){

    m_currentState = STATE_IDLE;
    m_previousState = STATE_IDLE;

    initIdleTransitions();
    initMovingTransitions();
    initJumpingTransitions();
    initCollideDownTransitions();
    initCollideUpTransitions();
    initCollideHorizontalTransitions();
}

void FSM::initIdleTransitions(){
    validIdleTransitions[0] = {EVENT_MOVE, STATE_MOVING};
    validIdleTransitions[1] = {EVENT_JUMP, STATE_JUMPING};
}

void FSM::initMovingTransitions(){
    validMovingTransitions[0] = {EVENT_NONE, STATE_IDLE};
    validMovingTransitions[1] = {EVENT_JUMP, STATE_JUMPING};
    validMovingTransitions[2] = {EVENT_COLLIDE_HORIZONTAL, STATE_COLLIDE_HORIZONTAL};
}

void FSM::initJumpingTransitions(){
    validJumpingTransitions[0] = {EVENT_COLLIDE_UP, STATE_COLLIDE_UP};
    validJumpingTransitions[1] = {EVENT_COLLIDE_DOWN, STATE_COLLIDE_DOWN};
    validJumpingTransitions[2] = {EVENT_COLLIDE_HORIZONTAL, STATE_COLLIDE_HORIZONTAL};
}

void FSM::initCollideUpTransitions()
{
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_JUMPING};
}


void FSM::initCollideDownTransitions()
{
    validCollideDownTransitions[0] = {EVENT_TIMER, STATE_IDLE};
}

void FSM::initCollideHorizontalTransitions()
{
    validCollideHorizontalTransitions[0] = {EVENT_TIMER, STATE_MOVING};
}


bool FSM::CheckValidTransition(State t_currentState, Event t_event)
{
    switch (t_currentState)
    {
        // For all states except collide Horizontal, they don't care about check for previous states
        // We only update previous states in what Horizontal collide need - moving and jumping

        case STATE_IDLE:
            for (int index = 0; index < m_numIdleTransitions; index++)
            {
                if (validIdleTransitions[index].first == t_event)
                {
                    m_currentState = validIdleTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;

        case STATE_MOVING:
            for (int index = 0; index < m_numMovingTransitions; index++)
            {
                if (validMovingTransitions[index].first == t_event)
                {
                    m_previousState = m_currentState;
                    m_currentState = validMovingTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;
        
        case STATE_JUMPING:
            for (int index = 0; index < m_numJumpingTransitions; index++)
            {
                if (validJumpingTransitions[index].first == t_event)
                {
                    m_previousState = m_currentState;
                    m_currentState = validJumpingTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;


        case STATE_COLLIDE_UP:
                if (validCollideUpTransitions[0].first == t_event)
                {
                    m_currentState = validCollideUpTransitions[0].second;
                    return true;
                    break;
                }
        break;

        case STATE_COLLIDE_DOWN:
            if (validCollideDownTransitions[0].first == t_event)
                {
                    m_currentState = validCollideDownTransitions[0].second;
                    return true;
                    break;
                }
        break;

        case STATE_COLLIDE_HORIZONTAL:
            if (validCollideHorizontalTransitions[0].first == t_event) 
            {
                m_currentState = validCollideHorizontalTransitions[0].second;
                return true;
                break;
            }
        break;
            
    }

    return false;
}