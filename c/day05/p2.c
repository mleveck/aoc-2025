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
  // merges an element with all subsequent elements (if any)
  // Then consider the subsequent elements that it didn't merge with

  // construct merged_ranges
  // allocate to size of ranges to give enough space, we'll adjust when done
  i64list merged_ranges =
      (i64list){.len = ranges.len, .data = new (perm, i64, ranges.len)};
  usize merged_size = 0;
  while (ranges.len) {
    // same with unmerged_ranges
    // made so that we can only consider ranges that weren't absorbed on next iteration
    i64list unmerged_ranges =
        (i64list){.len = ranges.len, .data = new (perm, i64, ranges.len)};
    usize unmerged_size = 0;
    i64 lower = ranges.data[0];
    i64 upper = ranges.data[1];
    for (usize j = 2; j < ranges.len; j += 2) {
      i64 lower2 = ranges.data[j];
      i64 upper2 = ranges.data[j + 1];
      if (can_merge(lower, upper, lower2, upper2)) {
        lower = lower < lower2 ? lower : lower2;
        upper = upper > upper2 ? upper : upper2;
        continue;
      } else {
        unmerged_ranges.data[unmerged_size] = lower2;
        unmerged_ranges.data[unmerged_size + 1] = upper2;
        unmerged_size += 2;
      }
    }
    merged_ranges.data[merged_size] = lower;
    merged_ranges.data[merged_size + 1] = upper;
    merged_size += 2;
    unmerged_ranges.len = unmerged_size;
    // only look at the unmerged ranges on next iteration
    ranges = unmerged_ranges;
  }
  merged_ranges.len = merged_size;
  return merged_ranges;
}

int main(int argc, char **argv) {
  if (argc > 1)
    INPUT_FNAME = argv[1];
  arena perm = arena_create((size)1024 * 1024 * 20);
  arena scratch = arena_create(1024 * 1024 * 4);
  s8 ftext = slurp(INPUT_FNAME, &perm);
  if (ftext.len < 0) {
    fprintf(stderr, "Couldn't open file %s\n", INPUT_FNAME);
    exit(1);
  }

  s8list lines = get_lines(ftext, &perm);

  // find the break between ranges and ingredients
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

  // construct the ranges
  i64 *range_data = new (&perm, i64, range_strs.len * 2);
  i64list ranges = (i64list){.len = range_strs.len * 2, .data = range_data};
  for (usize i = 0; i < range_strs.len; i++) {
    usize range_idx = i * 2;
    s8list bounds = split(range_strs.data[i], '-', &perm);
    ranges.data[range_idx] = to_long(bounds.data[0], scratch);
    ranges.data[range_idx + 1] = to_long(bounds.data[1], scratch);
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
    i64 lower = merged_ranges.data[i];
    i64 upper = merged_ranges.data[i + 1];
    fresh += (upper - lower) + 1;
  }
  printf("Fresh count %zu\n", fresh);

  return 0;
}
