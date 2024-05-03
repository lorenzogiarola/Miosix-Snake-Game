#pragma once

namespace Terminal {
	void SetupConsole();
	void SetupGame(unsigned short x, unsigned short y);
	void ResetTerminal();
	
	void PrintMessage(std::string message, bool err = false);
	void RemoveMessage();
	void PrintGameOver();
	void UpdateScore();
	void RemoveButtons();
	void PrintButtons(button buttons[], unsigned short size);
	
	void AddHead(unsigned short x, unsigned short y);
	void RemoveHead(unsigned short x, unsigned short y);
	void RemoveTail(unsigned short x, unsigned short y);
	void AddApple(unsigned short x, unsigned short y);
}
