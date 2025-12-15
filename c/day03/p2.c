#include "../util.h"
#include <stdio.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

int main(int argc, char **argv) {
  if (argc > 1)
    INPUT_FNAME = argv[1];
  arena perm = arena_create(1024 * 1024 * 40);
  s8 ftext = slurp(INPUT_FNAME, &perm);
  if (ftext.len < 0) {
    fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
    exit(1);
  }
  s8list lines = get_lines(ftext, &perm);
  size total_joltage = 0;
  for (usize lidx = 0; lidx < lines.len; lidx++) {
    s8 line = lines.data[lidx];
    i64 joltage = 0;
    i32 digits[12] = {0};
    for (usize d = 0; d < 12; d++) {
      digits[d] = -255;
    }
    usize start = 0;
    for (int f = 0; f < 12; f ++) {
      for (usize j = start; j < line.len - (11 - f); j++) {
        if (line.data[j] > digits[f] ) {
          digits[f] = line.data[j];
          start = j + 1;
        }
      }
    }
    for (usize z = 0; z < 12; z++) {
      joltage = joltage * 10 + digits[z] - '0';
    }
    total_joltage += joltage;
  }
  printf("Joltage: %td\n", total_joltage);
  return 0;
}
