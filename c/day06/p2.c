#include "../util.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

s8list rotate_left(s8list lines, arena *perm) {
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

s8ll parse_line_groups(s8list lines, arena *perm, arena scratch) {
    i64list cutpts = {.data = new (&scratch, i64, lines.len), .len = 0};
    for (usize i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        s8 stripped = stripws(line);
        if (0 == stripped.len) {
            cutpts.data[cutpts.len++] = i;
        }
    }
    s8ll line_groups = {.data = new (perm, s8list, cutpts.len + 1),
                        .len = cutpts.len + 1};
    usize start = 0;
    usize i;
    for (i = 0; i < cutpts.len; i++) {
        s8list line_group = {.data = &lines.data[start],
                             .len = cutpts.data[i] - start};
        line_groups.data[i] = line_group;
        start = cutpts.data[i] + 1;
    }
    // final line group
    s8list line_group = {.data = &lines.data[start], .len = lines.len - start};
    line_groups.data[i] = line_group;
    return line_groups;
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

i64 process(s8ll line_groups, s8 operators, arena scratch) {
    assert(line_groups.len == operators.len);
    i64 answer = 0;
    for (usize i = 0; i < line_groups.len; i++) {
        u8 operator= operators.data[i];
        s8list lgroup = line_groups.data[i];
        i64 result = to_long(stripws(lgroup.data[0]), scratch);
        for (usize j = 1; j < lgroup.len; j++) {
            i64 operand = to_long(stripws(lgroup.data[j]), scratch);
            switch (operator) {
            case '*':
                result = result * operand;
                break;
            case '+':
                result = result + operand;
                break;
            };
        }
        answer += result;
    }
    return answer;
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
    s8 operators = parse_operators(lines, &perm);
    reverse_str(operators);
    s8list rlines = rotate_left(lines, &perm);
    s8ll line_groups = parse_line_groups(rlines, &perm, scratch);
    i64 answer = process(line_groups, operators, scratch);

    printf("Answer: %lld\n", answer);
    return 0;
}
