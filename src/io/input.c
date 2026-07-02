#include "io/input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>
#endif

static int terminal_raw_mode = 0;

#ifdef _WIN32
#include <windows.h>
static HANDLE hstdin;
static DWORD old_mode;

void input_set_raw(int enable) {
    if (enable) {
        hstdin = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hstdin, &old_mode);
        SetConsoleMode(hstdin, old_mode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT));
        terminal_raw_mode = 1;
    } else {
        if (terminal_raw_mode) {
            SetConsoleMode(hstdin, old_mode);
            terminal_raw_mode = 0;
        }
    }
}
#else
static struct termios old_tio;

void input_set_raw(int enable) {
    if (enable) {
        struct termios new_tio;
        tcgetattr(STDIN_FILENO, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
        new_tio.c_cc[VMIN] = 1;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
        terminal_raw_mode = 1;
    } else {
        if (terminal_raw_mode) {
            tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
            terminal_raw_mode = 0;
        }
    }
}
#endif

int input_get_key(void) {
    if (!terminal_raw_mode) input_set_raw(1);

#ifdef _WIN32
    int ch = _getch();
    if (ch == 0 || ch == 0xE0) {
        ch = _getch();
        switch (ch) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
            default: return KEY_NONE;
        }
    }
    return ch;
#else
    unsigned char ch;
    if (read(STDIN_FILENO, &ch, 1) != 1) return KEY_NONE;

    if (ch == 27) {
        unsigned char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1) return 27;
        if (read(STDIN_FILENO, &seq[1], 1) != 1) return 27;

        if (seq[0] == '[') {
            switch (seq[1]) {
                case 'A': return 'w';
                case 'B': return 's';
                case 'C': return 'd';
                case 'D': return 'a';
                default: return KEY_NONE;
            }
        }
        return KEY_NONE;
    }

    return ch;
#endif
}

int input_get_key_timeout(int ms) {
    if (!terminal_raw_mode) input_set_raw(1);

#ifdef _WIN32
    HANDLE hstd = GetStdHandle(STD_INPUT_HANDLE);
    DWORD dwRead;
    INPUT_RECORD ir[16];
    DWORD dwEvents;
    while (1) {
        if (!GetNumberOfConsoleInputEvents(hstd, &dwEvents)) return KEY_NONE;
        if (dwEvents == 0) {
            Sleep(ms);
            return KEY_NONE;
        }
        ReadConsoleInput(hstd, ir, 1, &dwRead);
        if (dwRead == 0) return KEY_NONE;
        if (ir[0].EventType == KEY_EVENT && ir[0].Event.KeyEvent.bKeyDown) {
            int ch = ir[0].Event.KeyEvent.uChar.AsciiChar;
            if (ch) return ch;
        }
    }
#else
    struct timeval tv;
    tv.tv_sec = ms / 1000;
    tv.tv_usec = (ms % 1000) * 1000;

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);

    if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) <= 0)
        return KEY_NONE;

    return input_get_key();
#endif
}

void input_flush(void) {
#ifdef _WIN32
    HANDLE hstd = GetStdHandle(STD_INPUT_HANDLE);
    FlushConsoleInputBuffer(hstd);
#else
    tcflush(STDIN_FILENO, TCIFLUSH);
#endif
}

bool input_key_pressed(int key) {
    (void)key;
    return false;
}

const char *input_key_name(int key) {
    switch (key) {
        case 'w': return "UP";
        case 's': return "DOWN";
        case 'a': return "LEFT";
        case 'd': return "RIGHT";
        case '.': return "WAIT";
        case 'g': return "GET";
        case 'i': return "INVENTORY";
        case '?': return "HELP";
        case 'Q': return "QUIT";
        case '>': return "STAIRS DOWN";
        case '<': return "STAIRS UP";
        default: return "UNKNOWN";
    }
}
