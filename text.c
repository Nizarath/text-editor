#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text.h"
#include "state.h"
#include "config.h"

int total_lines(line *text)
{
    int n = 0;
    while (text != NULL) {
        n++;
        text = text->next;
    }
    return n;
}

void free_text(line *text)
{
    line *next;
    while (text != NULL) {
        next = text->next;
        free(text);
        text = next;
    }
}

void str_replace(char *s, int old, int new)
{
    for (int i = 0; s[i]; i++)
        if (s[i] == old)
            s[i] = new;
}

line *read_text(FILE *f)
{
    line *head = NULL;
    line **cur;
    char buf[LINE_SIZE];

    for (cur = &head; fgets(buf, sizeof buf, f) != NULL;
            cur = &((*cur)->next)) {
        // replace all tabs (editor does not understand them)
        str_replace(buf, '\t', ' ');
        //
        *cur = malloc(sizeof (line));
        strcpy((*cur)->s, buf);
        (*cur)->next = NULL;
    }

    // check empty file
    if (head == NULL) {
        head = calloc(1, sizeof (line));
        head->s[0] = '\n';
    }

    return head;
}

// test
void print_text(line *text)
{
    int n = 0;
    while (text != NULL) {
        printf("%p %d: %s", text, n+1, text->s);
        text = text->next;
        n++;
    }
}

line *get_line(line *text, int n)
{
    while (n--)
        text = text->next;
    return text;
}

line *get_cur_line(state *st)
{
    return get_line(st->text, st->start_line + st->curs_row);
}


line *copy_line(line *l)
{
    line *copy = malloc(sizeof *copy);
    strcpy(copy->s, l->s);
    return copy;
}

line *copy_text(line *text)
{
    line *head = NULL;
    line **copy = &head;

    for (; text != NULL; text = text->next) {
        *copy = copy_line(text);
        (*copy)->next = NULL;
        copy = &((*copy)->next);
    }

    return head;
}

// append text, the following holds:
// - max size is LINE_SIZE (with '\0') or NCOLS+1
// - maximum 1 newline in that text
// - empty text is allowed
line *append(line *list, char *text)
{
    if (*text == '\0')
        return list;

    // find last line
    line *cur_line = list;
    while (cur_line->next != NULL)
        cur_line = cur_line->next;

    // get params
    int n_cur = strlen(cur_line->s);
    int n_text = strlen(text);

    // case 1 -- '\n' there
    if (n_cur && cur_line->s[n_cur-1]=='\n') {
        line *new_line = calloc(1, sizeof(*new_line));
        strcpy(new_line->s, text);
        cur_line->next = new_line;
        // case 2 -- there is a place to fit
    } else if (N_COLS - n_cur >= n_text) {
        strcat(cur_line->s, text);
    } else { // case 3 -- partly there, partly separate
        strncpy(cur_line->s + n_cur, text, N_COLS - n_cur);
        line *new_line = calloc(1, sizeof(*new_line));
        strcpy(new_line->s, text + N_COLS-n_cur);
        cur_line->next = new_line;
    }

    return list;
}


// free all added if fail on any level
// be careful to do merge with different lists, e.g.
// not merge(list1, list1->next), because this will
// lead to infinite merging
line *merge(line *list1, line *list2)
{
    line *cur_line;

    for (cur_line = list2; cur_line != NULL; cur_line = cur_line->next) {
        append(list1, cur_line->s);
    }

    return list1;
}
