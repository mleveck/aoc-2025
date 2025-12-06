#include "../util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

g64 parse_nums(s8list lines, arena *perm, arena scratch) {
    usize n_num_rows = lines.len - 1; // last line is operators
    g64 grid = {.data = new (perm, i64list, n_num_rows), .len = n_num_rows};
    for (usize i = 0; i < n_num_rows; i++) { 
        s8list number_tokens = splitws(lines.data[i], perm);
        i64list row = {.data = new (perm, i64, number_tokens.len),
                       .len = number_tokens.len};
        for (usize j = 0; j < number_tokens.len; j++) {
            i64 num = to_long(number_tokens.data[j], scratch);
            row.data[j] = num;
        }
        grid.data[i] = row;
    }
    return grid;
}

s8 parse_operators(s8list lines, arena *perm) {
    // tokenize last line which is operators
    s8list tokens = splitws(lines.data[lines.len - 1], perm);
    s8 operators = {.data = new (perm, u8, tokens.len), .len = tokens.len};
    for (usize i = 0; i < tokens.len; i++) {
        operators.data[i] = tokens.data[i].data[0];
    }
    return operators;
}

int main(int argc, char **argv) {
    if (argc > 1)
        INPUT_FNAME = argv[1];
    arena perm = arena_create(1024 * 1024 * 4);
    arena scratch = arena_create(1024 * 1024 * 4);
    s8 ftext = slurp(INPUT_FNAME, &perm);
    if (ftext.len < 0) {
        fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
        exit(1);
    }
    s8list lines = get_lines(ftext, &perm);
    g64 grid = parse_nums(lines, &perm, scratch);
    s8 operators = parse_operators(lines, &perm);
    usize nrows = grid.len;
    usize ncols = grid.data[0].len;

    i64 answer = 0;
    for (usize c = 0; c < ncols; c++) {
        u8 operator = operators.data[c];
        i64 result = grid.data[0].data[c];
        for (usize r = 1; r < nrows; r++) {
            i64 operand2 = grid.data[r].data[c];
            switch (operator) {
                case '*':
                    result = result * operand2;
                    break;
                case '+':
                    result = result + operand2;
                    break;
            };
        }
        answer += result;
    }
    printf("Answer: %lld\n", answer);
    return 0;
}
