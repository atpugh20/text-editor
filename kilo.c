/**
* This program is based on the text-editor tutorial located at:
* https://viewsourcecode.org/snaptoken/kilo/03.rawInputAndOutput.html.
* 
* This project helped strengthen my skills in the C programming language,
* terminal manipulation, and memory management. 
*/

/*** INCLUDES ***/

#include <asm-generic/ioctls.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

/*** DEFINES ***/

// Code shortcut for using CTRL key with another key (k) 
#define CTRL_KEY(k) ((k) & 0x1f)

/*** DATA ***/

struct editor_config {
    int screen_rows;
    int screen_cols;
    struct termios orig_termios;  // Original terminal attributes for exit   
};

struct editor_config E;

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
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1) {
        die("tcsetattr");   
    }
}

void enable_raw_mode() {
    /** 
    * Makes changes to terminal settings to enter raw editor mode.
    */

    // Disable on program termination
    if (tcgetattr(STDIN_FILENO, &E.orig_termios) == -1) {
        die("tcgetattr");
    }
    atexit(disable_raw_mode);

    struct termios raw = E.orig_termios;
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

int get_cursor_position(int *rows, int *cols) {
    /** 
    * 
    */
    char buf[32];
    unsigned int i = 0;
    
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return -1;

    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';

    if (buf[0] != '\x1b' || buf[1] != '[') return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return -1;
    
    return -1;
}

int get_window_size(int *rows, int *cols) {
    /**
    * Updates the rows and cols parameters with the terminal width and height
    */
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        // If ioctl fails, move to bottom of terminal manually
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) {
            return -1;
        }
        return get_cursor_position(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}

/*** APPEND BUFFER ***/

struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

/*** OUTPUT ***/

void editor_draw_rows() {
    /**
    * Draws the row data for the opened file.
    */
    int y;
    for (y = 0; y < E.screen_rows; y++) {
        write(STDOUT_FILENO, "~", 1);

        if (y < E.screen_rows - 1) {
            write(STDOUT_FILENO, "\r\n", 2);
        }
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

void init_editor() {
    /** 
    * Initializes all the fields in the E editor config struct.
    */
    if (get_window_size(&E.screen_rows, &E.screen_cols) == -1) {
        die("get_window_size");
    }
}

int main() {
    // Initialize the editor
    enable_raw_mode();
    init_editor();

    // Editor loop
    while (1) {
        editor_refresh_screen();
        editor_process_keypress();
    }

    return 0;
}
