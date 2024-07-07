#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

// Sets the terminal input back to the original
void disableRawMode() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Gets the terminal's input as raw, flips the bits of raw to 0, then sets the input to raw (0)
void enableRawMode() {
	tcgetattr(STDIN_FILENO, &orig_termios); // sets original termios
	atexit(disableRawMode); // will disable raw on the termination of the program
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO | ICANON); // reads bit by bit
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 
}

int main() {
	enableRawMode();
  char c;
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q');
	return 0;
}