#include "../util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void swap(i64list* old, i64list* new) {
    i64list temp = *old;
    *old = *new;
    *new = temp;
    memset(new->data, 0, sizeof(i64) * new->len);
}

u8 gets8(s8 str, usize idx) {
    if (idx >= 0 && idx < str.len) {
        return str.data[idx];
    }
    return '.';
}

void  set_s(i64list path_counts, s8 line) {
    for (usize i = 0; i < line.len; i++) {
        if (line.data[i] == 'S') {
            path_counts.data[i] = 1;
            return;
        }
    }
    fprintf(stderr, "Could not find S in initial line\n");
    exit(1);
}

i64 process(s8list lines, arena scratch) {
    usize ncols = lines.data[0].len;
    i64list prev_path_counts = {.data = new (&scratch, i64, ncols),
                                .len = ncols};
    i64list new_path_counts = {.data = new (&scratch, i64, ncols),
                               .len = ncols};
    set_s(prev_path_counts, lines.data[0]); // checked and S is on 1st line in
                                            // sample input and actual input
    for (usize i = 1; i < lines.len; i++) {
        s8 line = lines.data[i];
        for (usize col = 0; col < line.len; col++) {
            u8 c = line.data[col];
            if ('.' == c) {
                new_path_counts.data[col] = prev_path_counts.data[col];
                if (gets8(line, col - 1) == '^') {
                    new_path_counts.data[col] += prev_path_counts.data[col - 1];
                }
                if (gets8(line, col + 1) == '^') {
                    new_path_counts.data[col] += prev_path_counts.data[col + 1];
                }
            }
        }
        swap(&prev_path_counts, &new_path_counts);
    }
    return sum(prev_path_counts);
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024 * 1024 * 40);
    arena scratch = arena_create(1024 * 1024 * 4);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    i64 answer = process(lines, scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
