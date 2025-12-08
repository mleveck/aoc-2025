#include "../util.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct b { // Not the greatest name give bools are b32. But b means
                   // (junction) box.
    i64 x, y, z;
} b;

typedef struct blist {
    b *data;
    size len;
} blist;

typedef struct bpair {
    i64 idx1, idx2, dist;
} bpair;

typedef struct bpl {
    bpair *data;
    size len;
} bpl;

blist parse_input(s8list lines, arena *perm, arena scratch) {
    blist boxes = {.data = new (perm, b, lines.len), .len = lines.len};
    for (usize row = 0; row < lines.len; row++) {
        s8 line = lines.data[row];
        s8list toks = split(line, ',', perm);
        assert(3 == toks.len);
        b box = {.x = to_long(toks.data[0], scratch),
                 .y = to_long(toks.data[1], scratch),
                 .z = to_long(toks.data[2], scratch)};
        boxes.data[row] = box;
    }
    return boxes;
}

void append_bp(bpl *box_pairs, bpair box_pair) {
    box_pairs->data[box_pairs->len++] = box_pair;
}

i64 calc_dist(b box1, b box2) {
    // we only care about relative distances, so no need for sqrt
    i64 dx = box1.x - box2.x;
    i64 dy = box1.y - box2.y;
    i64 dz = box1.z - box2.z;
    return dx * dx + dy * dy + dz * dz;
}

bpl gen_box_pairs(blist boxes, arena *perm) {
    size npairs = combinations(boxes.len, 2);
    bpl box_pairs = {.data = new (perm, bpair, npairs), .len = 0};
    for (size b1_idx = 0; b1_idx < boxes.len - 1; b1_idx++) {
        b box1 = boxes.data[b1_idx];
        for (size b2_idx = b1_idx + 1; b2_idx < boxes.len; b2_idx++) {
            b box2 = boxes.data[b2_idx];
            i64 dist = calc_dist(box1, box2);
            bpair pair = {.idx1 = b1_idx, .idx2 = b2_idx, .dist = dist};
            append_bp(&box_pairs, pair);
        }
    }
    assert(box_pairs.len == npairs);
    return box_pairs;
}

int cmp_bpairs(const void *pair1_ptr, const void *pair2_ptr) {
    bpair *pair1 = (bpair *)pair1_ptr;
    bpair *pair2 = (bpair *)pair2_ptr;
    if (pair1->dist < pair2->dist) {
        return -1;
    }
    if (pair2->dist < pair1->dist) {
        return 1;
    }
    return 0;
}

i64list assign_circuits(bpl box_pairs, i64 n_jbs, i64 n_pairs_to_connect,
                        arena *perm) {
    i64list circuit_assignments = {.data = new (perm, i64, n_jbs),
                                   .len = n_jbs};
    i64 max_circuit_id = 1;
    for (usize i = 0; i < n_pairs_to_connect; i++) {
        bpair box_pair = box_pairs.data[i];
        i64 circuit_id;
        i64 ca1 = circuit_assignments.data[box_pair.idx1];
        i64 ca2 = circuit_assignments.data[box_pair.idx2];
        if (ca1 && ca2) {
            if (ca1 == ca2) {
                continue;
            }
            i64 min_circuit_id = ca1 < ca2 ? ca1 : ca2;
            i64 max_circuit_id = ca1 > ca2 ? ca1 : ca2;
            circuit_id = min_circuit_id;

            for (usize k = 0; k < circuit_assignments.len; k++) {
                if (max_circuit_id == circuit_assignments.data[k]) {
                    circuit_assignments.data[k] = circuit_id;
                }
            }
        } else if (ca1) {
            circuit_id = ca1;
        } else if (ca2) {
            circuit_id = ca2;
        } else {
            circuit_id = max_circuit_id++;
        }
        circuit_assignments.data[box_pair.idx1] = circuit_id;
        circuit_assignments.data[box_pair.idx2] = circuit_id;
    }
    return circuit_assignments;
}

int cmp_counter(const void *c1_ptr, const void *c2_ptr) {
    i64 c1 = *(i64 *)c1_ptr;
    i64 c2 = *(i64 *)c2_ptr;
    if (c1 < c2) { // we want max sort
        return 1;
    }
    if (c2 < c1) {
        return -1;
    }
    return 0;
}

i64 process(i64list circuit_assignments, arena scratch) {
    i64list counter = {.data = new (&scratch, i64, circuit_assignments.len),
                       .len = circuit_assignments.len};
    for (usize i = 0; i < circuit_assignments.len; i++) {
        i64 ca = circuit_assignments.data[i];
        if (ca)
            counter.data[ca] += 1;
    }
    qsort(counter.data, counter.len, sizeof(i64), cmp_counter);
    return product((i64list){.data = counter.data, .len = 3});
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 40);
    arena scratch = arena_create(1024L * 400);
    s8 input_text = read_input(argc, argv, &perm);

    i64 npairs_to_connect =
        argc < 2 ? 10 : 1000; // input file arg => real data. else sample
    s8list lines = get_lines(input_text, &perm);
    blist boxes = parse_input(lines, &perm, scratch);
    bpl box_pairs = gen_box_pairs(boxes, &perm);
    qsort(box_pairs.data, box_pairs.len, sizeof(bpair), cmp_bpairs);
    i64list circuit_assignments =
        assign_circuits(box_pairs, boxes.len, npairs_to_connect, &perm);
    i64 answer = process(circuit_assignments, scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
