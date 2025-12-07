#include "../util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

b32 in(s8 str, u8 c) {
    for (usize i = 0; i < str.len; i++) {
        if (str.data[i] == c) {
            return 1;
        }
    }
    return 0;
}

b32 ini64(i64list list, i64 num) {
    for (usize i = 0; i < list.len; i++) {
        if (list.data[i] == num) {
            return 1;
        }
    }
    return 0;
}

void add(i64list* idxs, i64 idx) {
    if (!ini64(*idxs, idx)) {
        idxs->data[idxs->len++] = idx;
    }
}

i64 process(s8list lines, arena *perm, arena scratch) {
    i64 nsplits = 0;
    i64list prev_beam_idxs =  {.data = new (&scratch, i64, lines.data[0].len),
                                 .len = 0};
    for (usize i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        i64list new_beam_idxs = {.data = new (&scratch, i64, lines.data[i].len),
                                 .len = 0};
        for (usize j = 0; j < line.len; j++) {
            u8 c = line.data[j];
            switch (c) {
            case 'S':
                add(&new_beam_idxs, j);
                break;
            case '^':
                if (ini64(prev_beam_idxs, j)) {
                    add(&new_beam_idxs, j - 1);
                    add(&new_beam_idxs, j + 1);
                    nsplits++;
                }
                break;
            case '.':
                if (ini64(prev_beam_idxs, j)) {
                    add(&new_beam_idxs, j);
                }
                break;
            default:
                fprintf(stderr,
                        "HIT AN UNHANDLED CHAR: %c on line: %zu col: %zu\n", c,
                        i, j);
                exit(1);
            }
        }
        prev_beam_idxs = new_beam_idxs;
    }
    return nsplits;
}

int main(int argc, char **argv) {
    if (argc > 1)
        INPUT_FNAME = argv[1];
    arena perm = arena_create(1024 * 1024 * 40);
    arena scratch = arena_create(1024 * 1024 * 4);
    s8 ftext = slurp(INPUT_FNAME, &perm);
    if (ftext.len < 0) {
        fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
        exit(1);
    }

    s8list lines = get_lines(ftext, &perm);
    i64 answer = process(lines, &perm, scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
