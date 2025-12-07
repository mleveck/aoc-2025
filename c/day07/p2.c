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

void add(i64list *idxs, i64 idx) { idxs->data[idxs->len++] = idx; }

void printi64list(i64list list) {
    for (usize i = 0; i < list.len; i++) {
        printf("%lld", list.data[i]);
    }
    printf("\n");
}

i64 sum(i64list list) {
    i64 res = 0;
    for (usize i = 0; i < list.len; i++) {
        res += list.data[i];
    }
    return res;
}

i64 geti64(i64list list, usize idx) {
    if (idx >= 0 && idx < list.len) {
        return list.data[idx];
    }
    return 0;
}

u8 gets8(s8 str, usize idx) {
    if (idx >= 0 && idx < str.len) {
        return str.data[idx];
    }
    return '.';
}

i64 process(s8list lines, arena *perm, arena scratch) {
    i64 npaths = 0;
    usize ncols = lines.data[0].len;
    i64ll path_counts = {.data = new (&scratch, i64list, lines.len),
                         .len = lines.len};
    path_counts.data[0] =
        (i64list){.data = new (&scratch, i64, ncols), .len = ncols};
    for (usize i = 1; i < lines.len; i++) {
        s8 line = lines.data[i];
        i64list new_path_counts = {.data = new (&scratch, i64, ncols),
                                   .len = ncols};

        for (usize j = 0; j < line.len; j++) {
            u8 c = line.data[j];
            switch (c) {
            case 'S':
                break;
            case '^':
                break;
            case '.':
                if (lines.data[i - 1].data[j] == 'S') {
                    new_path_counts.data[j] = 1;
                    break;
                }
                new_path_counts.data[j] = path_counts.data[i - 1].data[j];
                if (gets8(line, j - 1) == '^') {
                    new_path_counts.data[j] += path_counts.data[i - 1].data[j -1];
                }
                if (gets8(line, j + 1) == '^') {
                    new_path_counts.data[j] += path_counts.data[i - 1].data[j  + 1];
                }
                break;
            default:
                fprintf(stderr,
                        "HIT AN UNHANDLED CHAR: %c on line: %zu col: %zu\n", c,
                        i, j);
                exit(1);
            }
        }
        printi64list(new_path_counts);
        path_counts.data[i] = new_path_counts;
    }
    i64list final_path_counts = path_counts.data[path_counts.len -1];

    for (usize i = 0; i < final_path_counts.len; i++) {
        npaths += final_path_counts.data[i];
    }
    return npaths;
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
