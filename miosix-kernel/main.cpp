#include <iostream>
#include <random>
#include "Definitions.h"
#include "Terminal.h"
#include "InputHandler.h"

#define WAIT_TIME 75 //0.08

/*  GLOBAL VAR  */
unsigned short matrix[WIDTH][HEIGHT];	//Field of game matrix[COLUMN][ROW]
unsigned short lenght = 1;
position head = { WIDTH / 2, HEIGHT / 2 };	//Head position
position tail = { head.x, head.y };	//Tail position
position apple;	//apple position
bool gameOver = false;
bool keepPlay = true;

#pragma region UTILITY FUNCTIONS
unsigned short NextX(unsigned short current, short delta) {
    int newVal = current + delta;

    if (newVal < 0) {
        newVal += WIDTH;
    }
    else if (newVal >= WIDTH) {
        newVal = newVal % WIDTH;
    }

    return newVal;
}

unsigned short NextY(unsigned short current, short delta) {
    int newVal = current + delta;

    if (newVal < 0) {
        newVal = HEIGHT + newVal;
    }
    else if (newVal >= HEIGHT) {
        newVal = newVal % HEIGHT;
    }

    return newVal;
}

static void MaxNumberReset() {
    int tmp = 0; //Otherwise possible OVERFLOW problems with just "matrix[i][k] += lenght - USHRT_MAX"
    for (int i = 0; i < WIDTH; i++) {
        for (int k = 0; k < HEIGHT; k++) {
            if (matrix[i][k] != 0) {
                tmp = matrix[i][k];
                matrix[i][k] =(unsigned) (tmp + lenght - USHRT_MAX);
            }
        }
    }
}
#pragma endregion

#pragma region GAMEPLAY
position NewApplePosition() {
    position newPos = { (unsigned short)(rand() % WIDTH), (unsigned short)(rand() % HEIGHT) };
    
    if (matrix[newPos.x][newPos.y] > 0) { //Collide with the snake, start searching for a new position:
        unsigned short x = rand() % WIDTH;
        unsigned short y = 0;

        for (int i = 0; i < WIDTH; i++) {
            x = NextX(x, 1);
            y = rand() % HEIGHT;

            for (int k = 0; k < HEIGHT; k++) {
                y = NextY(y, 1);

                if (matrix[x][y] == 0) { //New possible position in (x,y)
                    newPos = { x, y };
                    goto endSearch; //End searching exiting the for loop
                }
            }
        }
    }
    endSearch:
    return newPos;
}

void EatApple() {
    lenght++;
    
    apple = NewApplePosition();
    Terminal::AddApple(apple.x, apple.y);
    
    Terminal::UpdateScore();
}

static bool MoveHead(unsigned short x, unsigned short y) {
    bool shouldMoveTail = true;

    //Check for collision:
    if (matrix[x][y] > 0) {
        gameOver = true;
        return false;
    }
    else if (apple.x == x && apple.y == y) {
        EatApple();
        shouldMoveTail = false;
    }

    //Remove old rendered head:
    Terminal::RemoveHead(head.x, head.y);

    //Add new head in the matrix and update pointer
    matrix[x][y] = matrix[head.x][head.y] + 1;
    head = { x, y };

    //Render new head:
    Terminal::AddHead(head.x, head.y);

    if (matrix[head.x][head.y] == USHRT_MAX) {
        MaxNumberReset();
    }

    return shouldMoveTail;
}

static void MoveTail() {
    //Reset old tail position in the matrix to zero
    unsigned short newTailVal = matrix[tail.x][tail.y] + 1;
    matrix[tail.x][tail.y] = 0;

    //Remove rendered tail:
    Terminal::RemoveTail(tail.x, tail.y);

    //Update the new tail position
    if (matrix[tail.x][NextY(tail.y, 1)] == newTailVal) {
        //New tail up
        tail.y = NextY(tail.y, 1);
    }
    else if (matrix[NextX(tail.x, 1)][tail.y] == newTailVal) {
        //New tail right
        tail.x = NextX(tail.x, 1);
    }
    else if (matrix[tail.x][NextY(tail.y, -1)] == newTailVal) {
        //New tail down
        tail.y = NextY(tail.y, -1);
    }
    else if (matrix[NextX(tail.x, -1)][tail.y] == newTailVal) {
        //New tail left
        tail.x = NextX(tail.x, -1);
    }
    else {
        //TODO error
    }
}

static bool Move(command dir) {
    switch (dir)
    {
    case UP:
        return MoveHead(head.x, NextY(head.y, -1));

    case LEFT:
        return MoveHead(NextX(head.x, -1), head.y);

    case DOWN:
        return MoveHead(head.x, NextY(head.y, 1));

    case RIGHT:
        return MoveHead(NextX(head.x, 1), head.y);

    default: //ERR
        Terminal::PrintMessage("Error in the choosen movement direction ", true);
        return false;
    }
}
#pragma endregion


#pragma region MENU
/*
unsigned short generic menu(buttons[], lenght){} that returns the chioce number and handle input

bool specific menu(){} that create buttons, call generic menu and returns the correct value

enum with the position of the menu? (Bottom, Top, Middle...)
*/

unsigned short GenericMenu(button buttons[], unsigned short size) {
    command c = NOCOMMAND;
    int selected = 0;

    for (int i = 0; i < size; i++) {
        if (buttons[i].selected) {
            selected = i;
            break;
        }
    }

    //PrintButtons:
    Terminal::PrintButtons(buttons, size);

    //Handle Selection:
    while (c != ENTER) {
        c = InputHandler::GetInput();

        if (c == RIGHT) {
            //Update Selected index and bool in the array
            buttons[selected].selected = false;
            selected = (selected + 1) % size;
            buttons[selected].selected = true;

            //Update Rendered choice
            Terminal::PrintButtons(buttons, size);
        }
        else if (c == LEFT) {
            //Update Selected index and bool in the array
            buttons[selected].selected = false;
            selected = selected -1;
            if (selected < 0) selected += size;
            buttons[selected].selected = true;

            //Update Rendered choice
            Terminal::PrintButtons(buttons, size);
        }

        miosix::Thread::sleep(100);
    }
    
    //Return selected index:
    return (unsigned short) selected;
}

bool ContinuePlayingMenu() {
    bool continuePlaying = false;

    button buttons[2];
    buttons[0] = { "YES", true };
    buttons[1] = { "NO", false };

    //Ask if the player wants to continue playing:
    Terminal::PrintMessage("Do you want to play again?");

    if (GenericMenu(buttons, 2) == 0) { //YES -> Keep playing
        continuePlaying = true;
    }

    //Remove Rendered question:
    Terminal::RemoveButtons();

    //InputHandler scelta

    return continuePlaying;
}

void PauseGame() {
    command c = NOCOMMAND;
    Terminal::PrintMessage("GAME PAUSED, press any key to resume");

    while (c == NOCOMMAND) c = InputHandler::GetInput();

    Terminal::RemoveMessage();
}
#pragma endregion

static void Setup() {
    //SETUP MATRIX, HEAD AND TAIL
    lenght = 1;

    //ClearMatrix:
    for (int i = 0; i < WIDTH; i++) {
        for (int k = 0; k < HEIGHT; k++) {
            matrix[i][k] = 0;
        }
    }

    head = { WIDTH / 2, HEIGHT / 2 };
    tail = { head.x, head.y };

    matrix[head.x][head.y] = 1;

    //SETUP APPLE:
    apple = NewApplePosition();

    //SETUP SCREEN:
    Terminal::SetupGame(head.x, head.y);
    Terminal::AddApple(apple.x, apple.y);
}

int main()
{
    keepPlay = true;
    command input, lastInput = UP;
    float waitTime = 0;

    InputHandler::Setup();
    Terminal::SetupConsole();
    while (InputHandler::GetInput() == NOCOMMAND); //Wait for a command

    while (keepPlay)
    {
        //SETUP
        Setup();

        //GAME:
        while (!gameOver)
        {
            //INPUT:
            input = InputHandler::GetInput();

            switch (input)
            {
            case UP: case DOWN: case LEFT: case RIGHT:
                lastInput = input;
                break;
            case OTHER: case NOCOMMAND:
                input = lastInput;
                break;
            case ESC:
                PauseGame();
                input = lastInput;
                break;
            default:
                input = lastInput;
                break;
            }

            //UPDATE:
            if (Move(input)) { //Need to move tail
                MoveTail();
            }

            std::cout.flush();

            //SLEEP: based on the direction, to account for difference in height/width of the single cell
            waitTime = (input == RIGHT || input == LEFT) ? (float) WAIT_TIME : (float) (2.0 * WAIT_TIME);
            miosix::Thread::sleep(waitTime);
        }
        Terminal::PrintGameOver();

        gameOver = false;

        //KEEP PLAYING?
        keepPlay = ContinuePlayingMenu();
    }

    //RESET TERMINAL AND CLOSE

    Terminal::ResetTerminal();
}
