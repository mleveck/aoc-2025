#include "../util.h"
#include <stdio.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

b32 check_repeats(s8 cand, s8 rest){
  if (rest.len < cand.len) return 0;
  for (usize i =0; i < cand.len; i++) {
    if (cand.data[i] != rest.data[i]) {
      return 0;
    }
  }
  if (cand.len == rest.len) return 1;
  return check_repeats(cand, slice(rest, cand.len, rest.len));
}

b32 find_repeats(s8 digits) {
  if (digits.len > 1) {
    char firstdigit = digits.data[0];
    for (usize sidx = 1; sidx < digits.len; sidx++) {
      if (digits.data[sidx] == firstdigit){
        if (check_repeats(slice(digits, 0, sidx), slice(digits, sidx, digits.len))){
          return 1;
        }
      }
    }
  }
  return 0;
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
  s8list range_strs = split(lines.list[0], ',', &perm);
  i64 bad_code_sum = 0;
  char buf[1000] = {0};
  for (usize i = 0; i < range_strs.len; i++) {
    s8list range = split(range_strs.list[i], '-', &perm);
    i64 start = to_long(range.list[0], scratch);
    i64 end = to_long(range.list[1], scratch);
    for (usize j = start; j <= end; j++) {
      sprintf(buf, "%zu", j);
      s8 digits = tos8(buf, &perm);
      if (find_repeats(digits)) bad_code_sum += j;
    }
  }
  printf("Bad Code Sum: %lld\n", bad_code_sum);
  return 0;
}
