#ifndef _EDITOR_CONFIG_H
#define _EDITOR_CONFIG_H

// Set your OS here:
#define UNIX

// set terminal parameters here:
#define N_TERM_ROWS     24
#define N_TERM_COLS     80

#define N_ROWS          (N_TERM_ROWS-2)
#define N_COLS          (N_TERM_COLS-1)

#define LINE_SIZE       (N_COLS+1)

// set maximum number of undos
#define N_UNDOS         100

// set how many spaces TAB generates
#define N_TAB           4

void raw_mode();
void canonic_mode();
int input_char();

#endif
