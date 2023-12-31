#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "state.h"
#include "selection.h"

selection *init_selection()
{
    selection *sel = malloc(sizeof *sel);
    sel->is_started = 0;
    return sel;
}

void free_selection(selection *sel)
{
    free(sel);
}

void start_selection(state *st, selection *sel)
{
    // load start coordinate
    sel->first_start_line = st->start_line;
    sel->first_curs_row = st->curs_row;
    sel->first_curs_col = st->curs_col;
    // mark currently expanded
    sel->is_started = 1;
}

void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

// happens either
// - other action taken
// - selection action finished (copied or deleted)
void stop_selection(state *st, selection *sel)
{
    if (sel->is_started == 0) { // no selection -- then just select this char
        sel->first_start_line = sel->last_start_line = 
            st->start_line;
        sel->first_curs_row = sel->last_curs_row =
            st->curs_row;
        sel->first_curs_col = sel->last_curs_col = 
            st->curs_col;
        return;
    }

    // load stop coordinate
    sel->last_start_line = st->start_line;
    sel->last_curs_row = st->curs_row;
    sel->last_curs_col = st->curs_col;
    // determine order (whether expanded forward or backward)
    int first = sel->first_start_line + sel->first_curs_row;
    int last = sel->last_start_line + sel->last_curs_row;

    if ((first > last) || 
        (first == last && sel->first_curs_col > sel->last_curs_col)) {
        swap(&sel->first_start_line, &sel->last_start_line);
        swap(&sel->first_curs_row, &sel->last_curs_row);
        swap(&sel->last_curs_row, &sel->last_curs_row);
    }
    sel->is_started = 0;
}

