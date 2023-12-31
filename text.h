#ifndef _EDITOR_TEXT_H
#define _EDITOR_TEXT_H

#include <stdio.h>
#include "config.h"

typedef struct state state;

typedef struct line {
    char s[LINE_SIZE];
    struct line *next;
} line;

line *copy_line(line *);
line *copy_text(line *);
int total_lines(line *);
line *read_text(FILE *);
void free_text(line *);
void print_text(line *);
line *get_line(line *, int);
line *get_cur_line(state *st);
line *append(line *, char *);
line *merge(line *, line *);

#endif
