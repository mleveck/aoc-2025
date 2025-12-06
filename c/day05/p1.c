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


  usize cut_point = -1;
  for (usize i = 0; i < lines.len; i++) {
    if (lines.data[i].len == 0) {
      cut_point = i;
      break;
    }
  }

  if (cut_point == -1) {
    puts("could not find cut point");
    exit(1);
  }

  s8list range_strs = (s8list){.len = cut_point, .data = lines.data};
  s8list ingredients = (s8list){.len = lines.len - cut_point,
                                .data = &lines.data[cut_point + 1]};

  i64 *range_data = new (&perm, i64, range_strs.len * 2);
  i64list ranges = (i64list){.len = range_strs.len * 2, .data = range_data};
  for (usize i = 0; i < range_strs.len; i++) {
    usize range_idx = i * 2;
    s8list bounds = split(range_strs.data[i], '-', &perm);
    ranges.data[range_idx] = to_long(bounds.data[0], scratch);
    ranges.data[range_idx + 1] = to_long(bounds.data[1], scratch);
  }

  i64 fresh = 0;
  for (usize j = 0; j < ingredients.len; j++) {
    i64 ingredient = to_long(ingredients.data[j], scratch);
    for (usize k = 0; k < ranges.len; k += 2) {
      i64 lower = ranges.data[k];
      i64 upper = ranges.data[k + 1];
      if (ingredient >= lower && ingredient <= upper) {
        fresh++;
        break;
      }
    }
  }
  printf("Fresh count %lld\n", fresh);
  return 0;
}
