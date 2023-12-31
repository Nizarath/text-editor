#ifndef _EDITOR_STATE_H
#define _EDITOR_STATE_H

#include <stdio.h>
#include "text.h"

typedef struct line line;

typedef struct state {
    line *text;
    int start_line;
    int curs_row;
    int curs_col;
    int is_dirty;
    FILE *file;
    char *fname;

    struct state *prev;
    struct state *next;
} state;

state *init_state(line *, FILE *, char *);
void free_state(state *);
void free_state_all(state *);
void store_old_state(state *);

#endif
