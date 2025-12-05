#include "../util.h"
#include <stdio.h>
#include <string.h>

static char *INPUT_FNAME = "./sample_input.txt";

b32 can_merge(i64 l, i64 u, i64 l2, i64 u2) {
  if (l >= l2 && l <= u2) {
    return 1;
  }
  if (u >= l2 && u <= u2) {
    return 1;
  }
  if (l2 >= l && l2 <= u) {
    return 1;
  }
  if (u2 >= l && u2 <= u) {
    return 1;
  }
  return 0;
}

i64list merge_helper(i64list ranges, arena* perm) {
  i64list merged_ranges =
      (i64list){.len = ranges.len, .list = new (perm, i64, ranges.len)};
  usize merged_size = 0;
  usize merged_idx = 0;
  while (ranges.len) {
    i64list to_be_merged_ranges =
        (i64list){.len = ranges.len, .list = new (perm, i64, ranges.len)};
    usize to_be_merged_size = 0;
    usize to_be_merged_idx = 0;
    i64 lower = ranges.list[0];
    i64 upper = ranges.list[1];
    for (usize j = 2; j < ranges.len; j += 2) {
      i64 lower2 = ranges.list[j];
      i64 upper2 = ranges.list[j + 1];
      if (can_merge(lower, upper, lower2, upper2)) {
        lower = lower < lower2 ? lower : lower2;
        upper = upper > upper2 ? upper : upper2;
        continue;
      } else {
        to_be_merged_ranges.list[to_be_merged_idx] = lower2;
        to_be_merged_ranges.list[to_be_merged_idx + 1] = upper2;
        to_be_merged_idx += 2;
        to_be_merged_size += 2;
      }
    }
    merged_ranges.list[merged_idx] = lower;
    merged_ranges.list[merged_idx + 1] = upper;
    merged_idx +=2;
    merged_size += 2;
    to_be_merged_ranges.len = to_be_merged_size;
    ranges = to_be_merged_ranges;
  }
  merged_ranges.len = merged_size;
  return merged_ranges;
}

int main(int argc, char **argv) {
  if (argc > 1)
    INPUT_FNAME = argv[1];
  arena perm = arena_create((size)1024 * 1024 * 10000);
  arena scratch = arena_create(1024 * 1024 * 4);
  s8 ftext = slurp(INPUT_FNAME, &perm);
  if (ftext.len < 0) {
    fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
    exit(1);
  }

  s8list lines = get_lines(ftext, &perm);

  usize cut_point = -1;
  for (usize i = 0; i < lines.len; i++) {
    if (lines.list[i].len == 0) {
      cut_point = i;
      break;
    }
  }

  if (cut_point == -1) {
    puts("could not find cut point");
    exit(1);
  }

  s8list range_strs = (s8list){.len = cut_point, .list = lines.list};

  // construct the ranges
  i64 *range_data = new (&perm, i64, range_strs.len * 2);
  i64list ranges = (i64list){.len = range_strs.len * 2, .list = range_data};
  for (usize i = 0; i < range_strs.len; i++) {
    usize range_idx = i * 2;
    s8list bounds = split(range_strs.list[i], '-', &perm);
    ranges.list[range_idx] = to_long(bounds.list[0], scratch);
    ranges.list[range_idx + 1] = to_long(bounds.list[1], scratch);
  }

  i64 pre_merge_len = ranges.len;
  // initial merge
  i64list merged_ranges = merge_helper(ranges, &perm);
  // keep trying to merge until nothing is merged
  while (merged_ranges.len != pre_merge_len){
    pre_merge_len = merged_ranges.len;
    merged_ranges = merge_helper(merged_ranges, &perm);
  }

  usize fresh = 0;
  for (usize i = 0; i < merged_ranges.len; i+=2){
    i64 lower = merged_ranges.list[i];
    i64 upper = merged_ranges.list[i + 1];
    fresh += (upper - lower) + 1;
  }
  printf("Fresh count %zu\n", fresh);

  return 0;
}
