#include "fsm.h"

FSM::FSM(){
    std::cout << "FSM Object Created\n";
    initialize();
}

FSM::~FSM(){
    std::cout << "FSM Object Destroyed\n";
}

void FSM::initialize(){
    activeCommand = COMMAND_NONE;
    newCommand = COMMAND_NONE;
    event = EVENT_NONE;
    state = STATE_IDLE;
    initIdleTransitions();
    initMovingTransitions();
    initJumpingTransitions();
}

void FSM::initIdleTransitions(){
    validIdleTransitions[0] = STATE_MOVING;
    validIdleTransitions[1] = STATE_JUMPING;
}

void FSM::initMovingTransitions(){
    validMovingTransitions[0] = STATE_IDLE;
    validMovingTransitions[1] = STATE_JUMPING;
}

void FSM::initJumpingTransitions(){
    validJumpingTransitions[0] = STATE_IDLE;
    validJumpingTransitions[1] = STATE_MOVING;
}

bool FSM::isCommandActive(Commands t_active){
    if(activeCommand == t_active){
        return true;
    }
    return false;
}

void FSM::pollInputs(){ // If command is not active, new active command
    if(IsKeyDown(KEY_UP)){
        if(!isCommandActive(COMMAND_MOVE_UP)){
            newCommand = COMMAND_MOVE_UP;
            if(state != STATE_MOVING){
                checkValidTransition(STATE_MOVING);
            }
        }
    }
    if(IsKeyDown(KEY_DOWN)){
        if(!isCommandActive(COMMAND_MOVE_DOWN)){
            newCommand = COMMAND_MOVE_DOWN;
            if(state != STATE_MOVING){
                checkValidTransition(STATE_MOVING);
            }
        }
    }
    if(IsKeyDown(KEY_LEFT)){
       if(!isCommandActive(COMMAND_MOVE_LEFT)){
            newCommand = COMMAND_MOVE_LEFT;
            if(state != STATE_MOVING){
                checkValidTransition(STATE_MOVING);
            }
        }
    }
    if(IsKeyDown(KEY_RIGHT)){
        if(!isCommandActive(COMMAND_MOVE_RIGHT)){
            newCommand = COMMAND_MOVE_RIGHT;
            if(state != STATE_MOVING){
                checkValidTransition(STATE_MOVING);
            }
        }
    }
    if(IsKeyPressed(KEY_SPACE)){
        if(!isCommandActive(COMMAND_JUMP)){
            newCommand = COMMAND_JUMP;
            if(state != STATE_JUMPING){
                checkValidTransition(STATE_JUMPING);
            }
        }
    }
    if(IsKeyUp(KEY_UP) && IsKeyUp(KEY_DOWN) &&
       IsKeyUp(KEY_LEFT) && IsKeyUp(KEY_RIGHT) &&
       IsKeyUp(KEY_SPACE)){
        if(!isCommandActive(COMMAND_NONE)){
            newCommand = COMMAND_NONE;
            checkValidTransition(STATE_IDLE);
        }
    }
}

void FSM::checkValidTransition(State t_state){
    switch (state){
        case STATE_IDLE:
            arraySize = 2; // Change this num if adding more transitions
            for(int i = 0; i < arraySize;i++){
                if(t_state == validIdleTransitions[i]){
                    newEvent();
                    newTransition();
                }
            }
        break;
        case STATE_MOVING:
            arraySize = 2; // Change this num if adding more transitions
            for(int i = 0; i < arraySize;i++){
                if(t_state == validMovingTransitions[i]){
                    newEvent();
                    newTransition();
                }
            }
        break;
        case STATE_JUMPING:
            arraySize = 2; // Change this num if adding more transitions
            for(int i = 0; i < arraySize;i++){
                if(t_state == validJumpingTransitions[i]){
                    newEvent();
                    newTransition();
                }
            }
        break;
    }
}

void FSM::newEvent(){
    switch(newCommand){
        case COMMAND_MOVE_UP:
            std::cout << "EVENT_MOVE\n";
            event = EVENT_MOVE;
        break;
        case COMMAND_MOVE_DOWN:
            std::cout << "EVENT_MOVE\n";
            event = EVENT_MOVE;
        break;
        case COMMAND_MOVE_LEFT:
            std::cout << "EVENT_MOVE\n";
            event = EVENT_MOVE;
        break;
        case COMMAND_MOVE_RIGHT:
            std::cout << "EVENT_MOVE\n";
            event = EVENT_MOVE;
        break;
        case COMMAND_JUMP:
            std::cout << "EVENT_JUMP\n";
            event = EVENT_JUMP;
        break;
        case COMMAND_NONE:
            std::cout << "EVENT_NONE\n";
            event = EVENT_NONE;
        break;
    }
}

void FSM::newTransition(){
    if(event == EVENT_MOVE){
        if(newCommand == COMMAND_MOVE_UP || newCommand == COMMAND_MOVE_DOWN ||
           newCommand == COMMAND_MOVE_LEFT || newCommand == COMMAND_MOVE_RIGHT){
            exitState();
            enterState(STATE_MOVING);
        }
    }
    else if (event == EVENT_JUMP && newCommand == COMMAND_JUMP){
        exitState();
        enterState(STATE_JUMPING);
    }
    else if(event == EVENT_NONE && newCommand == COMMAND_NONE){
        exitState();
        enterState(STATE_IDLE);
    }
}

void FSM::enterState(State t_state){
    event = EVENT_NONE;
    activeCommand = newCommand;
    switch(t_state){
        case STATE_IDLE:
            std::cout << "Entering STATE_IDLE\n";
            // Insert methods to execute upon entering idle state
        break;
        case STATE_MOVING:
            std::cout << "Entering STATE_MOVING\n";
            // Insert methods to execute upon entering moving state
        break;
        case STATE_JUMPING:
            std::cout << "Entering STATE_JUMPING\n";
            // Insert methods to execute upon entering jumping state
        break;
    }
    state = t_state;
}

void FSM::updateState(){
    switch(state){
        case STATE_IDLE:
        break;
        case STATE_MOVING:
            switch(activeCommand){
                case COMMAND_MOVE_UP:
                    // movePlayerUp();
                break;
                case COMMAND_MOVE_DOWN:
                    // movePlayerDown();
                break;
                case COMMAND_MOVE_LEFT:
                    // movePlayerLeft();
                break;
                case COMMAND_MOVE_RIGHT:
                    // movePlayerRight();
                break;
            }
        break;
        case STATE_JUMPING:
        break;
    }
}

void FSM::exitState(){
    switch(state){
        case STATE_IDLE:
            std::cout << "Exiting STATE_IDLE\n";
            // Insert methods to execute upon leaving idle state
        break;
        case STATE_MOVING:
            std::cout << "Exiting STATE_MOVING\n";
            // Insert methods to execute upon leaving moving state
        break;
        case STATE_JUMPING:
            std::cout << "Exiting STATE_JUMPING\n";
            // Insert methods to execute upon leaving jumping state
        break;
    }
}

void FSM::update(){
    pollInputs();
    updateState();
}