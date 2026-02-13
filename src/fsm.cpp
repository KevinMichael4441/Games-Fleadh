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
    validMovingTransitions[2] = {EVENT_COLLIDE_HORIZONTAL, STATE_COLLIDE_HORIZONTAL};
}

void FSM::initCollideUpTransitions()
{
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_JUMPING};
}


void FSM::initCollideDownTransitions()
{
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_IDLE};
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_MOVING};
}

void FSM::initCollideHorizontalTransitions()
{
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_JUMPING};
    validCollideUpTransitions[0] = {EVENT_TIMER, STATE_MOVING};
}


bool FSM::CheckValidTransition(State t_currentState, Event t_event)
{
    switch (t_currentState)
    {
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
                    m_currentState = validJumpingTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;


        case STATE_COLLIDE_UP:
            for (int index = 0; index < m_numCollideUpTransitions; index++)
            {
                if (validCollideUpTransitions[index].first == t_event)
                {
                    m_currentState = validCollideUpTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;

        case STATE_COLLIDE_DOWN:
            for (int index = 0; index < m_numCollideDownTransitions; index++)
            {
                if (validCollideDownTransitions[index].first == t_event)
                {
                    m_currentState = validCollideDownTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;

        case STATE_COLLIDE_HORIZONTAL:
            for (int index = 0; index < m_numCollideHorizontalTransitions; index++)
            {
                if (validCollideHorizontalTransitions[index].first == t_event)
                {
                    m_currentState = validCollideHorizontalTransitions[index].second;
                    return true;
                    break;
                }
            }
        break;
            
    }

    return false;
}

State FSM::getState()
{
    return m_currentState;
}
