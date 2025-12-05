#include "../util.h"
#include <stdio.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

u8 get_el(s8list grid, usize r, usize c) {
  if (r < grid.len && c < grid.list[0].len) {
    return grid.list[r].data[c];
  }
  return '.';
}

int count_neighbors(s8list grid, usize r, usize c) {
  int count = 0;
  for (int i = -1; i < 2; i++) {
    for (int j = -1; j < 2; j++) {
      u8 el = get_el(grid, r + i, c + j);
      if (!(i == 0 && j == 0) && '@' == el) {
        count++;
      }
    }
  }
  return count;
}

int main(int argc, char **argv) {
  if (argc > 1)
    INPUT_FNAME = argv[1];
  arena perm = arena_create(1024 * 1024 * 40);
  s8 ftext = slurp(INPUT_FNAME, &perm);
  s8 ftext2 = slurp(INPUT_FNAME, &perm); //cheasy way to get an actual copy
  if (ftext.len < 0) {
    fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
    exit(1);
  }
  s8list grid = get_lines(ftext, &perm);
  s8list grid_dup = get_lines(ftext2, &perm);
  usize nrows = grid.len;
  usize ncols = grid.list[0].len;
  usize moveable;
  usize tot_moveable = 0;
  do {
    moveable = 0;
    for (usize r = 0; r < nrows; r++) {
      for (usize c = 0; c < ncols; c++) {
        u8 el = get_el(grid, r, c);
        grid_dup.list[r].data[c] = el;
        if ('@' == el) {
          int nn = count_neighbors(grid, r, c);
          if (nn < 4) {
            grid_dup.list[r].data[c] = '.';
            moveable++;
          }
        }
      }
    }
    tot_moveable += moveable;
    s8list tmp_grid = grid;
    grid = grid_dup;
    grid_dup = tmp_grid;
  } while (moveable > 0);
  printf("Total Movable count: %zu\n", tot_moveable);
  return 0;
}
