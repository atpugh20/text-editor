/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** DATA ***/

struct termios orig_termios;

/*** TERMINAL ***/

// Prints an error message and exits the program
void die(const char *s) {
  perror(s);
  exit(1);
}

// Sets the terminal input back to the original
void disableRawMode() {
	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) 
    die("tcsetattr");
}

// Gets the terminal's input as raw, flips the bits of raw to 0, then sets the input to raw (0)
void enableRawMode() {
	
  if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) die("tcgetattr");
	atexit(disableRawMode);
	struct termios raw = orig_termios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // fixes ctrlM, turns off software flow control,
  raw.c_oflag &= ~(OPOST); // turns off output processing
  raw.c_cflag |= (CS8);
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG); // reads bit by bit, sets these to 0: Echo, canonical mode, ctrlV, SIGINT/SIGSTP
  raw.c_cc[VMIN] = 0; // min bytes before read() can return
  raw.c_cc[VTIME] = 1; // max time to wait before read() returns

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) die("tcsetattr");
}

/*** INIT ***/

int main() {
	enableRawMode();
	while (1) {
    char c = '\0';
    if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) die("read");
    if (iscntrl(c)) {
      printf("%d\r\n", c); //%d converts
    } else {
      printf("%d ('%c')\r\n", c, c);
    }
    if (c == 'q') break;
  } 
  return 0;
}