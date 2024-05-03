#pragma once

#include "miosix.h"

#define HEIGHT 20
#define WIDTH 90
#define PINX 2
#define PINY 5

#define USHRT_MAX 65535


struct position {
	unsigned short x;
	unsigned short y;
};

struct button {
	std::string name;
	bool selected;
};

enum command { UP, DOWN, LEFT, RIGHT, OTHER, NOCOMMAND, ENTER, ESC};
