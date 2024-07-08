#include <ctype.h>
#include <stdio.h>
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
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // fixes ctrlM, turns off software flow control,
  raw.c_oflag &= ~(OPOST); // turns off output processing
  raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // reads bit by bit, sets these to 0: Echo, canonical mode, ctrlV, SIGINT/SIGSTP,
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); 
}

int main() {
	enableRawMode();
  char c;
	while (read(STDIN_FILENO, &c, 1) == 1 && c != 'q'){
    // if c is a control character (ASCII codes 0-31)
    if(iscntrl(c)) {
      printf("%d\r\n", c); //%d converts
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
  } 
  return 0;
}