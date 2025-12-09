#include "../util.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


typedef struct {
    i64 x;
    i64 y;
} tile;

typedef struct {
    tile* data;
    size len;
} tlist;

tlist new_tlist (usize cap, usize len, arena *a){
    tlist tiles;
    tiles.data = new(a, tile, cap);
    tiles.len = len;
    return tiles;
};

void append_tile(tlist *ts, tile t) {
    ts->data[ts->len++] = t;
}


tlist parse_input(s8list lines, arena *perm, arena scratch){
    i64 nrows = lines.len;
    tlist tiles = new_tlist(nrows, 0, perm);
    for(usize r = 0; r < nrows; r++){
        s8 line = lines.data[r];
        s8list toks = split(line, ',', &scratch);
        assert(2 == toks.len);
        i64 x = to_long(toks.data[0], scratch);
        i64 y = to_long(toks.data[1], scratch);
        append_tile(&tiles, (tile){.x=x,.y=y});
    }
    assert(tiles.len == lines.len);
    return tiles;
}

i64 calc_sq_size(tile t1, tile t2) {
    i64 width = llabs(t1.x - t2.x) + 1;
    i64 height = llabs(t1.y - t2.y) + 1;
    return width * height;
}

i64 process(tlist tiles) {
    i64 max_area = LLONG_MIN;
    for (usize ti1 = 0; ti1 < tiles.len -1; ti1++){
        for(usize ti2 = ti1 + 1; ti2 < tiles.len; ti2++) {
            tile t1 = tiles.data[ti1];
            tile t2 = tiles.data[ti2];
            i64 dist = calc_sq_size(t1, t2);
            max_area = dist > max_area ? dist: max_area;
        }
    }
    return max_area;
}
int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 40);
    arena scratch = arena_create(1024L *  40);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    tlist tiles = parse_input(lines, &perm, scratch);
    i64 answer = process(tiles);
    printf("Answer: %lld\n", answer);
    return 0;
}
