#ifndef _EDITOR_EDIT_H
#define _EDITOR_EDIT_H

#include "state.h"

void edit(state *);

/* different keycodes */
enum keycodes{
    KEY_UP = 23,    // ^W
    KEY_DOWN = 19,  // ^S
    KEY_LEFT = 1,   // ^A
    KEY_RIGHT = 4,  // ^D
    KEY_QUIT = 27,  // ESC
    KEY_SAVE = 12,  // ^L (load to file)
    KEY_NEXT_PAGE = 2, // ^B (VB is closest unused pair)
    KEY_PREV_PAGE = 22,// ^V (VB is closest unused pair)
    KEY_COPY = 3,   // ^C
    KEY_PASTE = 16, // ^P
    KEY_UNDO = 21,  // ^U
    KEY_REDO = 18,  // ^R
    KEY_FIND = 6,   // ^F
    KEY_DELETE = 24, // ^X (extract)
    KEY_GROUP = 7, // ^G
    KEY_HELP = 8,    // ^H
    KEY_TAB = 9,     // TAB
    KEY_BEG_LINE = 17, // ^Q (close to WASD)
    KEY_END_LINE = 5, // ^E (close to WASD)
    KEY_DEL_LINE = 26, // ^Z (near DELETE)
    KEY_BACKSPACE = 127,   // BACKSPACE
    KEY_GOTO_LINE = 14, // ^N (line Num)
    KEY_BEG_FILE = 20,  // ^T (TY is closest unused pair)
    KEY_END_FILE = 25    // ^Y (TY is closest unused pair)
};

#endif
