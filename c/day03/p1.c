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
  size joltage = 0;
  for (usize lidx = 0; lidx < lines.len; lidx++) {
    s8 line = lines.data[lidx];
    int max_highdig = -255;
    size max_highdig_idx = -1;
    int max_lowdig = -255;
    for (usize j = 0; j < line.len - 1; j++) {
      if (line.data[j] > max_highdig) {
        max_highdig = (int)line.data[j];
        max_highdig_idx = j;
      }
    }
    for (usize k = max_highdig_idx + 1; k < line.len; k++) {
      if (line.data[k] > max_lowdig) {
        max_lowdig = (int)line.data[k];
      }
    }
    joltage += (max_highdig - '0') * 10 + (max_lowdig - '0');
  }
  printf("Joltage: %td\n", joltage);
  return 0;
}
