#include <iostream>
#include "Definitions.h"
#include "Terminal.h"
#include <unistd.h>
#include <termios.h>

/*	COLORS	*/
#define SCORE_C 50, 50, 255
#define BORDER_C 170, 170, 170
#define HEAD_C 255, 255, 0
#define BODY_C 0, 255, 0
#define APPLE_C 255, 0, 0
#define ERROR_C 238, 210, 2
#define BLACK_C 0, 0, 0
#define GREY_C 220,220,220
#define RED_C 255, 0, 0
#define YELLOW_C 255, 255, 153

/*	OFFSET	*/
#define BOARD_OFFSET_Y 4 //Must be > 0
#define BOARD_OFFSET_X 3
#define LAST_ROW HEIGHT + BOARD_OFFSET_Y + 2

/*	GRAPHICS	*/
#define HORIZONTAL_BORDER_G 'X'
#define VERTICAL_BORDER_G "XX"
#define HEAD_G "O"
#define BODY_G "o"
#define APPLE_G "@"

/*	EXTERN VAR	*/
extern unsigned short lenght;

/*	VARIABLES	*/
unsigned short currentLastRow = 1;
struct termios config{}, oldConfig{};

using namespace Terminal;


#pragma region ANSICODE FUNCTIONS

	static void HideCursor() {
		printf("\033[?25l");
	}

	static void ShowCursor() {
		printf("\033[?25h");
	}

	static void MoveCursor(int x, int y) {
		printf("\033[%d;%dH", y, x);
	}

	static void ClearPage() {
		printf("\033[2J");
	}

	static void ClearLine() {
		printf("\33[2K");
	}

	static void ColorBackground(short int R, short int G, short int B) {
		printf("\033[48;2;%d;%d;%dm", R, G, B);
	}

	static void ColorText(short int R, short int G, short int B) {
		printf("\033[38;2;%d;%d;%dm", R, G, B);
	}

	static void ResetColor() {
		printf("\033[m");
	}

#pragma endregion

#pragma region UTILITY FUNCTIONS
static void ClearScreen() {
	for (int i = 1; i < currentLastRow + 1; i++) {
		MoveCursor(1, i);
		ClearLine();
	}
	ClearPage();
}

static void PrintScore() {
	MoveCursor((WIDTH + BOARD_OFFSET_X)/ 2 - 3, 1);	//First Row, around middle
	ColorText(SCORE_C);
	printf("Score: %d\n\r", lenght);
	ResetColor();
}

static void PrintBorders() {
	std::string horizontalBorder = std::string(WIDTH + 4, HORIZONTAL_BORDER_G); //+4 for the angles of the vertical border

	ColorBackground(BORDER_C);
	
	//UpperBorder
	MoveCursor(1, BOARD_OFFSET_Y - 1);
	std::cout<< horizontalBorder + "\n\r";

	//VerticalBorder:
	for (int i = BOARD_OFFSET_Y; i < HEIGHT + BOARD_OFFSET_Y; i++) {
		//Left
		MoveCursor(1, i);
		printf(VERTICAL_BORDER_G); //Double Space

		//Right
		MoveCursor(WIDTH + 3, i); //+3 for the first 2 space of the left border and another one because it's start from 1
		printf(VERTICAL_BORDER_G "\n\r"); //Double Space
	}

	//BottomBorder:
	MoveCursor(1, HEIGHT + BOARD_OFFSET_Y);
	std::cout << horizontalBorder;

	//Need to print a character to not have weird effects in reseizing the window after changing the background
	ResetColor();
	ColorText(0, 0, 0); //BLACK
	printf(".\n\r"); //TODO to not bug the screen
	
	ResetColor();
	MoveCursor(1, LAST_ROW);

	currentLastRow = LAST_ROW;
}

#pragma region LOGO
static void PrintLogo() {
	std::string name[] = {
"  ______    ____  _____        _        ___  ____    ________ \n\r",
".' ____ \\  |_   \\|_   _|      / \\      |_  ||_  _|  |_   __  |\n\r",
"| (___ \\_|   |   \\ | |       / _ \\       | |_/ /      | |_ \\_|\n\r",
" _.____`.    | |\\ \\| |      / ___ \\      |  __'.      |  _| _ \n\r",
"| \\____) |  _| |_\\   |_   _/ /   \\ \\_   _| |  \\ \\_   _| |__/ |\n\r",
" \\______.' |_____|\\____| |____| |____| |____||____| |________|\n\r"
	}; //Lenght = 63; Height = 6

	std::string snake2[] = {
"       ---_ ......._-_--.\n\r",
"      (|\\ /      / /| \\  \\\n\r",
"      /  /     .'  -=-'   `.\n\r",
"     /  /    .'             )\n\r",
"   _/  /   .'        _.)   /\n\r",
"  / o   o        _.-' /  .'\n\r",
"  \\          _.-'    / .'*|\n\r",
"   \\______.-'//    .'.' \\*|\n\r",
"    \\|  \\ | //   .'.' _ |*|\n\r",
"     `   \\|//  .'.'_ _ _|*|\n\r",
"      .  .// .'.' | _ _ \\*|\n\r",
"      \\`-|\\_/ /    \\ _ _ \\*\\\n\r",
"       `/'\\__/      \\ _ _ \\*\\\n\r",
"      /^|            \\ _ _ \\*\n\r",
"     '  `             \\ _ _ \\\n\r"
	};


	//SNAKE TEXT
	MoveCursor(1, 5);
	for (int i = 0; i < 6; i++) {
		MoveCursor(1, 5 + i);
		std::cout << name[i];
	}

	//SNAKE SYMBOL
	for (int i = 0; i < 15; i++) {
		MoveCursor(70, 1 + i);
		std::cout << snake2[i];
	}

	//TEXT TO PLAY:
	MoveCursor(1, 13);
	printf("Press any key to start");
	
	currentLastRow = 16;
}
#pragma endregion

#pragma endregion

#pragma region TERMINAL SETUP

bool setUpTerminal() {

	if (!isatty(STDIN_FILENO)) { //checks whether the file descriptor refers to a terminal
		std::cerr << "Standard input is not a terminal.\n\r";
		return false; // failure
	}

	/* Save old terminal configuration. */
	if (tcgetattr(STDIN_FILENO, &oldConfig) == -1 || tcgetattr(STDIN_FILENO, &config) == -1) {
		std::cerr << "Cannot get terminal settings: %s.\n\r";
		return false; // failure
	}

	// non-canonical mode activated with ~ICANON
	// ~ISIG implies reading some special terminating key combinations to be read as normal input
	// ~ECHO does not echo out the input characters
	config.c_lflag &= ~(ICANON | ISIG | ECHO);

	config.c_cc[VMIN] = 0; //minimum number of characters for canonical read
	config.c_cc[VTIME] = 0; //timeout for non-canonical read

	// if the custom settings for the terminal cannot be set, it resets the default configuration saver previously
	if (tcsetattr(STDIN_FILENO, TCSANOW, &config) == -1) {
		tcsetattr(STDIN_FILENO, TCSANOW, &oldConfig);
		std::cerr << "Cannot set terminal settings: %s.\n\r";
		return false; // failure
	}

	std::cout.flush();

	return true;
}
#pragma endregion

namespace Terminal{

	void SetupConsole() {
		ClearScreen();
		HideCursor();

		while(!setUpTerminal()) { //Try to set the terminal every 5 seconds
			miosix::Thread::sleep(5000);
			ClearScreen();
			HideCursor();
		}

		PrintLogo();
		fflush(stdout);
	}
	
	void SetupGame(unsigned short x, unsigned short y) {
		ClearScreen();
		PrintScore();
		PrintBorders();

		AddHead(x, y);
	}

	void PrintMessage(std::string message, bool err) {
		MoveCursor(1, currentLastRow);
		if(err) ColorText(ERROR_C);
		std::cout << message + "\n\r";
		ResetColor();
	}

	void RemoveMessage() {
		MoveCursor(1, currentLastRow);
		printf("\33[2K\n");
	}

	void PrintGameOver() {
		std::string text[] = {
	"***********************************************************",
	"*    _____                         ____                  **",
	"*   / ____|                       / __ \\                 **",
	"*  | |  __  __ _ _ __ ___   ___  | |  | |_   _____ _ __  **",
	"*  | | |_ |/ _` | '_ ` _ \\ / _ \\ | |  | \\ \\ / / _ \\ '__| **",
	"*  | |__| | (_| | | | | | |  __/ | |__| |\\ V /  __/ |    **",
	"*   \\_____|\\__,_|_| |_| |_|\\___|  \\____/  \\_/ \\___|_|    **",
	"*                                                        **",
	"***********************************************************"
		};
		
		ColorBackground(YELLOW_C);
		ColorText(BLACK_C);

		for (int i = 0; i < 9; i++) {
			MoveCursor(WIDTH/2 - 30 + BOARD_OFFSET_X, (HEIGHT + BOARD_OFFSET_Y)/2 - 4 + i);
			std::cout << text[i];
		}

		ResetColor();
	}

	void UpdateScore() {
		PrintScore();
	}

	void PrintButtons(button buttons[], unsigned short size) {
		unsigned short currentOffset = 1;

		for (int i = 0; i < size; i++) {
			//Print button[i]:
			MoveCursor(currentOffset, currentLastRow + 1);
			if (buttons[i].selected) {
				ColorBackground(GREY_C);
				ColorText(BLACK_C);
				std::cout << '*'; //'*' see the selection without colors
			}
			else std::cout << ' ';

			std::cout << buttons[i].name + "    "; //4 spaces 
			ResetColor();

			currentOffset += buttons[i].name.length() + 4;
		}
		std::cout << "\n\r";
	}

	void RemoveButtons() {
		MoveCursor(1, currentLastRow);
		ClearLine();
		MoveCursor(1, currentLastRow + 1);
		ClearLine();
	}

	void AddHead(unsigned short x, unsigned short y) {
		MoveCursor(BOARD_OFFSET_X + x, BOARD_OFFSET_Y + y);
		ColorText(HEAD_C);
		printf(HEAD_G);
		ResetColor();
	}

	void RemoveHead(unsigned short x, unsigned short y) { //Change head with body
		MoveCursor(BOARD_OFFSET_X + x, BOARD_OFFSET_Y + y);
		ColorText(BODY_C);
		printf(BODY_G);
		ResetColor();
	}

	void RemoveTail(unsigned short x, unsigned short y) { //Change head with body
		MoveCursor(BOARD_OFFSET_X + x, BOARD_OFFSET_Y + y);
		ResetColor();
		printf(" ");
	}

	void AddApple(unsigned short x, unsigned short y) {
		MoveCursor(BOARD_OFFSET_X + x, BOARD_OFFSET_Y + y);
		ColorText(APPLE_C);
		printf(APPLE_G);
		ResetColor();
	}

	void ResetTerminal() {
		ClearScreen();
		ResetColor();
		MoveCursor(1, 1);
		ShowCursor();

		// Restore terminal settings after terminating the execution
		tcsetattr(STDIN_FILENO, TCSAFLUSH, &oldConfig);
		std::cout << "THANKS FOR PLAYING\n\r";
	}
}
