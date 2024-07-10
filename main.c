#pragma region /*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#pragma endregion
#pragma region /***  DEFINES ***/

#define CTRL_KEY(k) ((k) & 0x1f)

#pragma endregion
#pragma region /*** DATA ***/

struct termios orig_termios;

#pragma endregion
#pragma region /*** TERMINAL ***/

void die(const char *s) {
  // Prints an error message and exits the program

  perror(s);
  exit(1);
}

void disableRawMode() {
  // Sets the terminal input back to the original

	if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) 
    die("tcsetattr");
}

void enableRawMode() {
	/* Gets the terminal's input as raw, flips the bits 
     of raw to 0, then sets the input to raw (0)*/

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

char editorReadKey() {
  /* waits for one keypress, then returns it */

  int nread;
  char c;
  while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
    if (nread == -1 && errno != EAGAIN) die("read");
  }
  return c;
}
#pragma endregion
#pragma region /*** OUTPUT ***/

void editorRefreshScreen() {
  
}

#pragma endregion
#pragma region /*** INPUT ***/

void editorProcessKeypress() {
  /* waits for the keypress, then handles it */
  
  char c = editorReadKey();

  switch (c) {
    case CTRL_KEY('q'):
      exit(0);
      break;
  }
}
#pragma endregion
#pragma region /*** INIT ***/

int main() {
	enableRawMode();

  while (1) {
    editorProcessKeypress();
  }

  return 0;
}
#pragma endregion