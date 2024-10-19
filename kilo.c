/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** DATA ***/

struct termios orig_termios;  // Original terminal attributes for exit

/*** TERMINAL ***/

void die(const char* s) {
    /**
    * Prints an error message and exits the program. Used for error handling.
    */
    perror(s);
    exit(1);
}

void disable_raw_mode() {
    /** 
    * Sets the terminal attributes back to default.
    */
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");   
    }
}

void enable_raw_mode() {
    /** 
    * Makes changes to terminal settings to enter raw editor mode.
    */

    // Disable on program termination
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disable_raw_mode);

    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);

    // terminal flag changes
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON); // Disables: C-s, C-q,  Fixes: carriage return
    raw.c_oflag &= ~(OPOST);                                  // Turns off "\n" to "\r\n" translation
    raw.c_cflag |= (CS8);
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);          // Disables: echo, canon mode, C-c, C-z, C-v

    // Set read() timeout time
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;
        
    // Apply changes to terminal
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        die("tcsetattr");   
    }
}

/*** INIT ***/

int main() {
    enable_raw_mode();
    
    while (1) {
        char c = '\0';
        if (read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) {
            die("read");
        }
        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }

    return 0;
}
