#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "text.h"
#include "state.h"
#include "edit.h"
#include "config.h"

void exit_error(char *msg)
{
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        exit_error("Usage: ./editor <file>");
    }

    FILE *f = fopen(argv[1], "r");
    if (f == NULL && (f = fopen(argv[1], "w+")) == NULL) {
        exit_error("Cannot open file");
    }


    // read file content
    line *text = read_text(f);

    // configure editor state
    state *st = init_state(text, f, argv[1]);

    // ----- configure session
    raw_mode();

    // editing loop until quit
    edit(st);

    // ------ restore all
    fclose(f);
    free_state_all(st);

    canonic_mode();
    exit(0);
}
