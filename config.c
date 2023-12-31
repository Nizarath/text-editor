#include "config.h"
#ifdef UNIX
#include <termios.h>
#include <unistd.h>
#endif
#include <stdio.h>

void raw_mode()
{
#ifdef UNIX
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    // no:
    // - echoing
    // - buffering
    // - signal handling (^C, ^D, ^Z, ^\)
    t.c_lflag &= ~(ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

void canonic_mode()
{
#ifdef UNIX
    struct termios t;

    tcgetattr(STDIN_FILENO, &t);
    // return echoing, buffering and signals
    t.c_lflag |= (ECHO | ICANON | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &t);
#endif
}

int input_char()
{
#ifdef WINDOWS
    return getch();
#else
    return getchar();
#endif
}
