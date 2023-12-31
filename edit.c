#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "selection.h"
#include "text.h"
#include "config.h"
#include "state.h"
#include "edit.h"

void clear_screen()
{
    for (int i = 0; i < N_TERM_ROWS; i++)
        printf("\n");
}

int min(int a, int b)
{
    return (a < b) ? a : b;
}

int max(int a, int b)
{
    return (a > b) ? a : b;
}

int line_between(int line, int border1, int border2)
{
    return line >= min(border1, border2) && 
        line <= max(border1, border2);
}

void print_screen(state *st, selection *sel)
{
    clear_screen();
    line *start_line = get_line(st->text, st->start_line);
    line *cur_line = start_line;
    int i;
    for (i = 0; i < N_ROWS && cur_line != NULL; i++) {
        if (st->curs_row == i)
            putchar('>');
        else if (sel != NULL && sel->is_started && 
                line_between(st->start_line + i, 
                    sel->first_start_line + sel->first_curs_row, 
                    st->start_line + st->curs_row))
            putchar('|');
        else
            putchar(' ');

        printf("%s", cur_line->s);
        if (cur_line->s[strlen(cur_line->s)-1] != '\n')
            putchar('\n');
        cur_line = cur_line->next;
    }
    for (; i < N_ROWS; i++)
        putchar('\n');
    for (i = -1; i < st->curs_col; i++)
        putchar(' ');
    // print E if current char is EOL
    cur_line = get_cur_line(st);
    if (cur_line->s[st->curs_col] == '\n')
        printf("E\n");
    else
        printf("^\n");
    // status line
    printf("%s%d/%d,%d", st->is_dirty ? "N " : "",
            st->start_line+st->curs_row+1, 
            total_lines(st->text),
            st->curs_col+1);
}

void adjust_curs_col(state *st)
{
    line *cur_line = get_cur_line(st);
    if (st->curs_col >= strlen(cur_line->s))
        st->curs_col = strlen(cur_line->s)-1;
}

void move_up(state *st)
{
    if (st->curs_row != 0)
        st->curs_row--;
    else {
        if (st->start_line == 0)
            return;
        st->start_line--;
    }
    adjust_curs_col(st);
}

void move_down(state *st)
{
    line *cur_line = get_cur_line(st);
    if (cur_line->next == NULL)
        return;

    if (st->curs_row == N_ROWS-1)
        st->start_line++;
    else
        st->curs_row++;
    adjust_curs_col(st);
}

void move_left(state *st)
{
    if (st->curs_col != 0) {
        st->curs_col--;
    }
}

void move_right(state *st)
{
    line *cur_line = get_cur_line(st);
    if (st->curs_col != strlen(cur_line->s)-1)
        st->curs_col++;
}

void next_page(state *st)
{
    for (int i = 0; i < N_ROWS; i++)
        move_down(st);
}

void prev_page(state *st)
{
    for (int i = 0; i < N_ROWS; i++)
        move_up(st);
}

void next_curs_col(state *st)
{
    line *cur_line = get_cur_line(st);
    if (st->curs_col != strlen(cur_line->s)-1) {
        st->curs_col++;
    } else if (cur_line->next != NULL) {
        move_down(st);
        st->curs_col=0;
    }
}


// all insertion -- clear and simple
// NOTE: cannot modify input, it can be e.g.
// global copying buffer
void insert(state *st, line *input)
{
    line *cur_line, *next;
    char tmp[LINE_SIZE];

    if (input == NULL)
        return;
    // save state before any edit
    store_old_state(st); 
    // -----
    input = copy_text(input); // forget about initial input
    cur_line = get_cur_line(st);
    strcpy(tmp, cur_line->s + st->curs_col);
    append(input, tmp);
    next = cur_line->next;
    cur_line->next = NULL;
    cur_line->s[st->curs_col] = '\0';
    merge(cur_line, input);
    merge(cur_line, next);
    // clear
    free_text(input);
    free_text(next);
    // move cursor
    next_curs_col(st);
    // mark unsaved
    st->is_dirty = 1;
}


line *wrap_char(int c)
{
    line *text = calloc(1, sizeof(*text));
    text->s[0] = c;
    return text;
}

void insert_letter(state *st, int c)
{
    line *letter = wrap_char(c);
    insert(st, letter);
    free_text(letter);
}

void save_file(state *st)
{
    st->file = freopen(st->fname, "w+", st->file);
    for (line *cur_line = st->text; cur_line != NULL;
            cur_line = cur_line->next) {
        fputs(cur_line->s, st->file);
    }
    st->is_dirty = 0;
    fflush(st->file);
}

void goto_line(state *st)
{
    // canonic mode only to input number (edit and echo)
    canonic_mode();
    printf(" line: ");
    char s[LINE_SIZE];
    fgets(s, sizeof s, stdin);
    s[strlen(s)-1] = '\0';
    raw_mode(); // return

    // parse this num
    int num = atoi(s)-1;
    if (num < 0 || num >= total_lines(st->text)) {
        printf(" incorrect range");
        getchar();
    } else {
        st->start_line = num;
        st->curs_row = 0;
        adjust_curs_col(st);
    }  
}

void find_text(state *st)
{
    // canonic mode only to input string (editing and echo)
    canonic_mode();
    printf(" find: ");
    char s[LINE_SIZE];
    fgets(s, sizeof s, stdin);
    s[strlen(s)-1] = '\0';
    raw_mode();    // return

    int n_line = st->start_line + st->curs_row;

    // now search, considering it can be on border
    for (line *cur_line=get_cur_line(st); cur_line != NULL;
            cur_line = cur_line->next, n_line++) {
        int len = strlen(cur_line->s);
        char str_cat[LINE_SIZE * 2];
        strcpy(str_cat, cur_line->s);
        if (cur_line->next != NULL)
            strcat(str_cat, cur_line->next->s);
        char *found = strstr(str_cat, s);
        if (found != NULL) {
            int pos = found - str_cat;
            if (pos < len) { // beginning in first line
                st->start_line = n_line;
                st->curs_row = 0;
                st->curs_col = pos;
            } else {        // beginning in second file
                st->start_line = n_line+1;
                st->curs_row = 0;
                st->curs_col = pos-len;
            }
            // can stop here or continue
            print_screen(st, NULL);
            printf(" press n to go to next");
            if (getchar()!='n')
                break; 
        }
    }
}

// selection is not broken only
// - moving key is typed
// - quit/save typed
// - selection option (copy/delete) is
// typed -- then it will call stop()
// later
int is_break_selection(int c)
{
    int keys[] = {
        KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, 
        KEY_QUIT, KEY_SAVE, KEY_NEXT_PAGE, KEY_PREV_PAGE, 
        KEY_COPY, KEY_DELETE
    };

    for (int i = 0; i < sizeof(keys)/sizeof(keys[0]); i++)
        if (c == keys[i])
            return 0;
    return 1;
}

// in text st, copy selection sel to buffer copy_buf
void copy(state *st, selection *sel, line **copy_buf)
{
    if (*copy_buf != NULL) {    // prepare cleared buf
        free_text(*copy_buf);
        *copy_buf = NULL;
    }

    // easy of read
    int line1 = sel->first_start_line + sel->first_curs_row;
    int line2 = sel->last_start_line + sel->last_curs_row;
    int pos1 = sel->first_curs_col;
    int pos2 = sel->last_curs_col;

    // copy all to buffer
    line **first_line = copy_buf;
    for (int i = line1; i <= line2; i++) {
        *copy_buf = copy_line(get_line(st->text, i));
        (*copy_buf)->next = NULL;
        copy_buf = &((*copy_buf)->next);
    }

    // remove after in last node
    line *last_line = get_line(*first_line, line2 - line1);
    last_line->s[pos2 + 1] = '\0';

    // remove before in first node
    strcpy((*first_line)->s, (*first_line)->s + pos1);

    // merge
    line *next = (*first_line)->next;
    (*first_line)->next = NULL;
    merge(*first_line, next);
    free_text(next);
}

void delete(state *st, selection *sel)
{
    // easy of read
    int line1 = sel->first_start_line + sel->first_curs_row;
    int line2 = sel->last_start_line + sel->last_curs_row;
    int pos1 = sel->first_curs_col;
    int pos2 = sel->last_curs_col;

    // ---- store old state
    store_old_state(st);

    // remove after in last node (
    // be careful as it can be same line, so cannot overwrite that,
    // instead copy to another node and merge then)
    line *last_line = get_line(st->text, line2);
    line *fixed_last_line = copy_line(last_line);
    strcpy(fixed_last_line->s, fixed_last_line->s + pos2+1);
    fixed_last_line->next = last_line->next;

    // remove after in first node
    line *first_line = get_line(st->text, line1);
    first_line->s[pos1] = '\0';

    // remove all between
    for (line *cur_line = first_line->next; cur_line != last_line->next; ) {
        line *next = cur_line->next;
        free(cur_line);
        cur_line = next;
    }

    first_line->next = NULL;
    merge(first_line, fixed_last_line);
    free_text(fixed_last_line);

    // restore cursor
    st->start_line = sel->first_start_line;
    st->curs_row = sel->first_curs_row;
    st->curs_col = sel->first_curs_col;

    // mark dirty
    st->is_dirty = 1;

    // check line became empty
    line *cur_line = get_cur_line(st);
    if (cur_line->s[0]=='\0')
        cur_line->s[0]='\n';
}

void backspace(state *st)
{
    int cur_line_num = st->start_line + st->curs_row;

    // cannot remove only if on very beginning
    if (cur_line_num == 0 && st->curs_col == 0) 
        return;

    store_old_state(st);

    line *edit_line;
    // beginning -- remove end of previous
    if (st->curs_col == 0) {
        edit_line = get_line(st->text, cur_line_num-1);
        int n = strlen(edit_line->s);
        edit_line->s[n-1] = '\0'; 

        // coordinates
        move_up(st);
        st->curs_col = n-1;

    } else { // edit this line only
        edit_line = get_cur_line(st);
        strcpy(&edit_line->s[st->curs_col-1], &edit_line->s[st->curs_col]);

        // coordinates
        st->curs_col--;
    }

    // merge with lines after it
    line *next = edit_line->next;
    edit_line->next = NULL;
    merge(edit_line, next);
    free_text(next);

    st->is_dirty = 1;
}

void delete_line(state *st)
{
    store_old_state(st);

    // find current and next line in generic manner
    // (so that similar way to delete head)
    line **cur = &(st->text);
    int i;
    for (i = 0; i < st->start_line + st->curs_row; i++)
        cur = &((*cur)->next);
    line *next = (*cur)->next;
    
    // if there is one line total -- do not delete
    if (i == 0 && next == NULL) {
        strcpy((*cur)->s, "\n");
    } else { // ok -- multiple lines
        free(*cur);
        *cur = next;
        if (*cur == NULL) { // removed last line
            move_up(st);
        }
    }
    adjust_curs_col(st);

    st->is_dirty = 1;
}

void undo(state **st)
{
    if ((*st)->prev != NULL) {
        *st = (*st)->prev;
    }
}

void redo(state **st)
{
    if ((*st)->next != NULL) {
        *st = (*st)->next;
    }
}

void goto_beg_file(state *st)
{
    st->start_line = 0;
    st->curs_row = 0;
    adjust_curs_col(st);
}

void goto_end_file(state *st)
{
    st->start_line = total_lines(st->text)-1;
    st->curs_row = 0;
    adjust_curs_col(st);
}

void help()
{
    clear_screen();
    char *help[] = {
            "SCREEN:\n",
            "+--------+\n",
            ">  text  |\n",
            "|  text  |\n",
            "+-^------+\n",
            "+-stat---+\n",
            "\n",
            "STAT:\n",
            "A/B,C - line/total, position\n",
            "N - non saved\n",
            ">^ - positions (E means end of line)",
            "\n",
            "KEYBOARD:\n",
            "^W - UP\n",
            "^S - DOWN\n",
            "^A - LEFT\n",
            "^D - RIGHT\n",
            "^Q - LINE BEGINNING\n",
            "^E - LINE END\n",
            "ESC - QUIT\n",
            "^L - SAVE\n",
            "^V - PGNEXT\n",
            "^B - PGPREV\n",
            "^C - COPY\n",
            "^I - INSERT\n",
            "^U - UNDO\n",
            "^R - REDO\n",
            "^X - DELETE\n",
            "^Z - LINE REMOVE\n",
            "^G - SELECT GROUP OF LINES\n",
            "^T - BEGINNING OF FILE\n",
            "^Y - END OF FILE\n",
            "^F - FIND\n",
            "^N - GOTO LINE NUMBER\n",
            "^H - HELP\n",
            0
    };

    int page = 0;
    for (int n = 0; help[n]; n++) {
        printf("%s", help[n]);
        if (n == (page+1)*N_TERM_ROWS - (page+2) && help[n+1]) {
            printf("<press anything to continue>\n");
            page++;
            input_char();
            clear_screen();
        }
    }
    input_char();
}

void move_beg_line(state *st)
{
    st->curs_col = 0; 
}

void move_end_line(state *st)
{
    line *cur_line = get_cur_line(st);
    st->curs_col = strlen(cur_line->s)-1;
}

/* quit the program */
int quit(state *st, selection *sel, line *copy_buffer)
{
    int answer;
    int must_quit = 0;

    if (st->is_dirty) {
        printf(" file is unsaved, are you sure to quit? ");
        answer = getchar();
    }
    if (!st->is_dirty || answer=='y') {
        must_quit = 1;
        if (copy_buffer != NULL)
            free_text(copy_buffer);
        free_selection(sel);
        clear_screen(st);
    }

    return must_quit;
}

void edit(state *st)
{
    line *copy_buffer = NULL;
    selection *sel = init_selection();

    while (1) {
        print_screen(st, sel);
        int c = input_char();
        // skip selection if needed
        if (is_break_selection(c)) {
            sel->is_started = 0;
        }
        // main
        switch (c) {
            case KEY_QUIT:
                if (quit(st, sel, copy_buffer))
                    return;
                break;
            case KEY_UP:
                move_up(st);
                break;
            case KEY_DOWN:
                move_down(st);
                break;
            case KEY_LEFT:
                move_left(st);
                break;
            case KEY_RIGHT:
                move_right(st);
                break;
            case KEY_NEXT_PAGE:
                next_page(st);
                break;
            case KEY_PREV_PAGE:
                prev_page(st);
                break;
            case KEY_SAVE:
                save_file(st);
                break;
            case KEY_FIND:
                find_text(st);
                break;
            case KEY_GROUP: // will also reset selection
                            // if already started
                start_selection(st, sel);
                break;
            case KEY_COPY:
                stop_selection(st, sel);
                copy(st, sel, &copy_buffer);
                break;
            case KEY_PASTE:
                insert(st, copy_buffer);
                break;
            case KEY_DELETE:
                stop_selection(st, sel);
                delete(st, sel);
                break;
            case KEY_UNDO:
                undo(&st);
                break;
            case KEY_REDO:
                redo(&st);
                break;
            case KEY_HELP:
                help();
                break;
            case KEY_TAB:
                for (int i = 0; i < N_TAB; i++)
                    insert_letter(st, ' ');
                break;
            case KEY_BEG_LINE:
                move_beg_line(st);
                break;
            case KEY_END_LINE:
                move_end_line(st);
                break;
            case KEY_DEL_LINE:
                delete_line(st);
                break;
            case KEY_GOTO_LINE:
                goto_line(st);
                break;
            case KEY_BEG_FILE:
                goto_beg_file(st);
                break;
            case KEY_END_FILE:
                goto_end_file(st);
                break;
            case KEY_BACKSPACE:
                backspace(st);
                break;
            default:    // letter to input
                insert_letter(st, c);
                break;
        }
    }
}
