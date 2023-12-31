/* selection operations -- copying and deletion */
#ifndef _EDITOR_SELECTION_H
#define _EDITOR_SELECTION_H

#include "text.h"

// we have to keep both start_line + row,
// in order to be able to return there e.g. after deletion
typedef struct {
    // first position
    int first_start_line;
    int first_curs_row;
    int first_curs_col;
    // last position
    int last_start_line;
    int last_curs_row;
    int last_curs_col;
    // is active?
    int is_started;
} selection;

selection *init_selection();
void free_selection(selection *);
void start_selection(state *, selection *);
void stop_selection(state *, selection *);

#endif
