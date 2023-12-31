#include <stdio.h>
#include <stdlib.h>
#include "state.h"
#include "text.h"

state *init_state(line *text, FILE *f, char *fname)
{
    state *st = malloc(sizeof *st);
    st->text = text;
    st->start_line = 0;
    st->curs_row = 0;
    st->curs_col = 0;
    st->is_dirty = 0;
    st->file = f;
    st->fname = fname;
    st->next = st->prev = NULL;

    return st;
}

void free_state(state *st)
{
    free(st);
}

void free_state_all(state *st)
{
    // find start
    while (st->prev != NULL)
        st = st->prev;
    // now clear all
    while (st != NULL) {
        state *next = st->next;
        free_text(st->text);
        free(st);
        st = next;
    }
}

static void remove_exceeded_state(state *st)
{
    // walk backward
    while (st->prev)
        st = st->prev;
    // count total count
    state *head = st;
    int n;
    for (n = 0; st; st = st->next)
        n++;
    // too high? remove all exceeded states
    while (n > N_UNDOS+1) {
        state *next = head->next;
        // rm
        free_text(head->text);
        free(head);
        // iterate
        head = next;
        n--;
    }
    head->prev = NULL;
}

void store_old_state(state *st)
{
    state *old_st = malloc(sizeof *old_st);

    line *old_text = copy_text(st->text);
    // content
    old_st->text = old_text;
    // coordinates
    old_st->start_line = st->start_line;
    old_st->curs_row = st->curs_row;
    old_st->curs_col = st->curs_col;
    // update
    old_st->is_dirty = st->is_dirty;
    // file
    old_st->file = st->file;
    old_st->fname = st->fname;
    // links
    old_st->prev = st->prev;
    if (st->prev != NULL)
        st->prev->next = old_st;
    old_st->next = st;

    // update new links
    st->prev = old_st;

    // test if too much states
    remove_exceeded_state(st);
}
