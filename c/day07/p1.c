#include "../util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

b32 in_i64(i64list list, i64 num) {
    // O(n) but I don't care for this size of data
    for (usize i = 0; i < list.len; i++) {
        if (list.data[i] == num) {
            return 1;
        }
    }
    return 0;
}

void add(i64list *idxs, i64 idx) {
    if (!in_i64(*idxs, idx)) {
        idxs->data[idxs->len++] = idx;
    }
}

i64 process(s8list lines, arena scratch) {
    i64 nsplits = 0;
    i64list prev_beam_idxs = {.data = new (&scratch, i64, lines.data[0].len),
                              .len = 0};
    i64list new_beam_idxs = {.data = new (&scratch, i64, lines.data[0].len),
                             .len = 0};
    for (usize i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        for (usize j = 0; j < line.len; j++) {
            u8 c = line.data[j];
            switch (c) {
            case 'S':
                add(&new_beam_idxs, j);
                break;
            case '^':
                if (in_i64(prev_beam_idxs, j)) {
                    add(&new_beam_idxs, j - 1);
                    add(&new_beam_idxs, j + 1);
                    nsplits++;
                }
                break;
            case '.':
                if (in_i64(prev_beam_idxs, j)) {
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
        i64list temp = prev_beam_idxs;
        prev_beam_idxs = new_beam_idxs;
        new_beam_idxs = temp;
        memset(new_beam_idxs.data, 0, sizeof(i64) * new_beam_idxs.len);
        new_beam_idxs.len = 0;
    }
    return nsplits;
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024 *  40);
    arena scratch = arena_create(1024  * 4);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    i64 answer = process(lines, scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
