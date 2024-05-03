#include <iostream>
#include "Definitions.h"
#include <miosix.h>
#include "AnalogReadLib.h"

#define KEY_ARROW_CHAR 91 //224
#define KEY_UP 65 //72
#define KEY_DOWN 66 //80
#define KEY_LEFT 68 //75
#define KEY_RIGHT 67 //77
#define KEY_ENTER 13
#define KEY_ESC 27
#define KEY_BACKSPACE 127

command comm = NOCOMMAND;

int xJoystick, yJoystick;

extern bool keepPlay;

command getArrow(int c){
    switch (c)
    {
    case KEY_UP:
        return UP;

    case KEY_DOWN:
        return DOWN;

    case KEY_LEFT:
        return LEFT;

    case KEY_RIGHT:
        return RIGHT;
        
    default:
        return OTHER;
    }
}

command getNormalKey(int c){
    switch (c)
    {
    case 'w': case 'W':
        return UP;

    case 'a': case 'A':
        return LEFT;

    case 's': case 'S':
        return DOWN;

    case 'd': case 'D':
        return RIGHT;

    case KEY_ENTER:
        return ENTER;
    
    case KEY_BACKSPACE:
        return ESC;
    
    default:
        return OTHER;
    }        
}

void DetectKey() {
    int c = getchar();

    if(c == KEY_ESC){
        c = getchar();
        if(c == KEY_ARROW_CHAR){ // ARROW KEY
            c = getchar();
            comm = getArrow(c);
        }
        else{
            DetectKey();
        }
    }
    else{
        comm = getNormalKey(c);
    }    
}

void DetectKeyT(void *argv){
    while(keepPlay){
        DetectKey();
    }
}

void GetJoystickT(void *argv){
    while(keepPlay){
        xJoystick = Joystick::readChannel(PINX);
        yJoystick = Joystick::readChannel(PINY);
        miosix::Thread::sleep(50);
    }
}

command GetJoystick() {
        if(xJoystick < ADC_MAX_VALUE * 0.25f){ //LEFT
            return LEFT;
        }
        else if(xJoystick > ADC_MAX_VALUE * 0.75f){ //RIGHT
            return RIGHT;
        }
        else if(yJoystick < ADC_MAX_VALUE * 0.25f){ //UP
            return UP;
        }
        else if(yJoystick > ADC_MAX_VALUE * 0.75f){ //DOWN
            return DOWN;
        }
        else{ //NO COMMAND
            return NOCOMMAND;
        }
    }

namespace InputHandler {
    
    void Setup() {
        //SETUP pin for the joystick:
        Joystick::initJoystick(PINX, PINY);

        //create thread of input polling:
        miosix::Thread::create(DetectKeyT, 2048, 2, 0, miosix::Thread::JOINABLE);
        miosix::Thread::create(GetJoystickT, 2048, 1, 0, miosix::Thread::JOINABLE);
    }
    
    command GetInput() {
        if(comm != NOCOMMAND){ //INPUT from keyboard
            command tmp = comm;
            comm = NOCOMMAND;
            return tmp;
        }
        else{ //INPUT from Joystick
            return GetJoystick();
        }
        
    }
}