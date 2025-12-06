#include "../util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

s8list rotate_nums_left(s8list lines, arena *perm) {
    size nrows = lines.len - 1; // last line is operators
    size ncols = lines.data[0].len;
    s8list rltext = {.data = new (perm, s8, ncols), .len = ncols};
    for (size c = ncols - 1; c > -1; c--) {
        s8 line = {.data = new (perm, u8, nrows), .len = nrows};
        for (size r = 0; r < nrows; r++) {
            line.data[r] = lines.data[r].data[c];
        }
        rltext.data[ncols - 1 - c] = line;
    }
    return rltext;
}

void reverse(s8 str) {
    size start = 0;
    size end = str.len - 1;
    while (end > start) {
        u8 tmp = str.data[start];
        str.data[start] = str.data[end];
        str.data[end] = tmp;
        start++;
        end--;
    }
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
    arena perm = arena_create(1024 * 1024 * 400);
    arena scratch = arena_create(1024 * 1024 * 40);
    s8 ftext = slurp(INPUT_FNAME, &perm);
    if (ftext.len < 0) {
        fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
        exit(1);
    }
    s8list lines = get_lines(ftext, &perm);
    s8 operators = parse_operators(lines, &perm);
    reverse(operators);
    s8list rlines = rotate_nums_left(lines, &perm);

    usize rlines_idx = 0;
    i64 answer = 0;
    for (usize i = 0; i < operators.len; i++) {
        u8 operator= operators.data[i];
        s8list result_toks = splitws(rlines.data[rlines_idx++], &perm);
        i64 result = to_long(result_toks.data[0], scratch);
        s8list operand_str_toks = splitws(rlines.data[rlines_idx], &perm);
        while (operand_str_toks.len > 0 && rlines_idx < rlines.len) {
            s8 operand_str = operand_str_toks.data[0];
            i64 operand = to_long(operand_str, scratch);
            switch (operator) {
            case '*':
                result = result * operand;
                break;
            case '+':
                result = result + operand;
                break;
            };
            if (rlines_idx < rlines.len -1) {
                rlines_idx++;
            } else {
                break;
            }
            operand_str_toks = splitws(rlines.data[rlines_idx], &perm);
        }
        rlines_idx += 1;
        answer += result;
    }
    printf("Answer: %lld\n", answer);
    return 0;
}
