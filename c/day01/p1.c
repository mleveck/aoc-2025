#include "../util.h"
#include <stdio.h>

static char *INPUT_FNAME = "./sample_input.txt";

int main(int argc, char **argv) {
  if (argc > 1)
    INPUT_FNAME = argv[1];
  arena perm = arena_create(1024 * 1024 * 2);
  arena scratch = arena_create(1024 * 1024 * 2);
  s8 ftext = slurp(INPUT_FNAME, &perm);
  if (ftext.len < 0) {
    fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
    exit(1);
  }
  s8list lines = get_lines(ftext, &perm);
  s8 dirs = (s8){.len = lines.len, .data = new (&perm, u8, lines.len)};
  i64list magnitudes =
      (i64list){.len = lines.len, .data = new (&perm, i64, lines.len)};
  for (usize i = 0; i < lines.len; i++) {
    printf("Line len: %td, end char %c \n",  lines.data[i].len, lines.data[i].data[lines.data[i].len -1]);
    s8 line = lines.data[i];
    char dir = line.data[0];
    i64 magnitude = to_long(slice(line, 1, line.len), scratch);
    dirs.data[i] = dir;
    magnitudes.data[i] = magnitude;
  }
  for (usize i = 0; i < dirs.len; i++) {
    printf("dir: %c mag: %lld\n", dirs.data[i], magnitudes.data[i]);
  }

  i64 pos = 50;
  i64 num_zero = 0;
  for (usize i = 0; i < dirs.len; i++) {
    printf("initial pos: %lld\n", pos);
    printf("mag: %lld\n", magnitudes.data[i]);
    pos += dirs.data[i] == 'R' ? magnitudes.data[i] : -1 * magnitudes.data[i];
    pos = (pos + 100) % 100;
    if (0 == pos)
      num_zero++;
  }
  printf("Answer: %lld\n", num_zero);
  return 0;
}
