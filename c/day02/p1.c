#include "../util.h"
#include <stdio.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

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
  s8list range_strs = split(lines.data[0], ',', &perm);
  i64 bad_code_sum = 0;
  for (usize i = 0; i < range_strs.len; i++) {
    s8list range = split(range_strs.data[i], '-', &perm);
    i64 start = to_long(range.data[0], scratch);
    i64 end = to_long(range.data[1], scratch);
    for (usize j = start; j <= end; j++) {
      char buf[1000] = {0};
      sprintf(buf, "%zu", j);
      size num_digits;
      if (((num_digits = strlen(buf)) % 2) == 0) {
        usize k;
        for (k = 0; k < num_digits / 2; k++) {
          if (buf[k] != buf[num_digits / 2 + k]) {
            break;
          }
        }
        if (k == num_digits/2) {
          bad_code_sum += j;
        }
      }
    }
  }
  printf("Bad code sum: %lld\n", bad_code_sum);
  return 0;
}
