/**
* This program is based on the text-editor tutorial located at:
* https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html.
* 
* This project helped strengthen my skills in the C programming language,
* terminal manipulation, and memory management. 
*/


/*** INCLUDES ***/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

/*** DEFINES ***/

#define CTRL_KEY(k) ((k) & 0x1f)

/*** DATA ***/

struct termios orig_termios;  // Original terminal attributes for exit

/*** TERMINAL ***/

void die(const char* s) {
    /**
    * Used for error handling. Clears the terminal, prints an error message,
    * and exits the program.
    */
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
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

char editor_read_key() {
    /**
    * Waits for one keypress, then returns it.
    */
    int nread;
    char c;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1) {
        if (nread == -1 && errno != EAGAIN) {
            die("read");
        }   
    }
    return c;
}

/*** OUTPUT ***/

void editor_draw_rows() {
    int y;
    for (y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editor_refresh_screen() {
    /**
    * Clears the screen, then draws all the rows.
    */
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);
    editor_draw_rows();
    write(STDOUT_FILENO, "\x1b[H", 3);
}

/*** INPUT ***/

void editor_process_keypress() {
    /**
    * Waits for the keypress, then handles its case. 
    */
    char c = editor_read_key();
    switch (c) {
        case CTRL_KEY('q'):
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[H", 3);    
            exit(0); 
            break;
    }
}

/*** INIT ***/

int main() {
    enable_raw_mode();
    
    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }

    return 0;
}
