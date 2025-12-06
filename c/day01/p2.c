#include "../util.h"
#include <stdio.h>

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
  s8 dirs = (s8){.len = lines.len, .data = new (&perm, u8, lines.len)};
  i64list magnitudes =
      (i64list){.len = lines.len, .data = new (&perm, i64, lines.len)};
  for (usize i = 0; i < lines.len; i++) {
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
    printf("Startng at pos %lld\n", pos);
    char dir = dirs.data[i];
    i64 rotating_pos = pos;
    i64 magnitude = magnitudes.data[i];
    if (dir == 'L') {
      for (i64 j = 1; j <= magnitude; j++) {
        rotating_pos--;
        if (rotating_pos % -100 == 0) {
          num_zero++;
        }
      }
    } else {
      for (i64 j = 1; j <= magnitude; j++) {
        rotating_pos++;
        if (rotating_pos % 100 == 0) {
          num_zero++;
        }
      }
    }

    pos = (rotating_pos + 100) % 100;
  }
  printf("Answer: %lld\n", num_zero);
  return 0;
}
