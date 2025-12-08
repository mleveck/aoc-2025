#include "../util.h"
#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

i64 combinations(i64 n, i64 r) { // from Gemini
    if (r < 0 || r > n) {
        return 0; // Invalid input for combinations
    }
    if (r == 0 || r == n) {
        return 1; // Base case: nC0 or nCn is always 1
    }
    if (r > n / 2) { // Optimization: C(n,r) = C(n, n-r)
        r = n - r;
    }

    i64 res = 1;
    for (int i = 1; i <= r; ++i) {
        res = res * (n - i + 1) / i;
    }
    return res;
}

typedef struct jb {
    i64 x, y, z;
} jb;

typedef struct jblist {
    jb *data;
    size len;
} jblist;

typedef struct jbpair {
    i64 idx1, idx2;
    i64 dist;
} jbpair;

typedef struct jbpl {
    jbpair *data;
    size len;
} jbpl;

jblist parse_input(s8list lines, arena *perm, arena scratch) {
    jblist boxes = {.data = new (perm, jb, lines.len), .len = lines.len};
    for (usize row = 0; row < lines.len; row++) {
        s8 line = lines.data[row];
        s8list toks = split(line, ',', perm);
        assert(3 == toks.len);
        jb box = {.x = to_long(toks.data[0], scratch),
                  .y = to_long(toks.data[1], scratch),
                  .z = to_long(toks.data[2], scratch)};
        boxes.data[row] = box;
    }
    return boxes;
}

void append_jbp(jbpl *box_pairs, jbpair box_pair) {
    box_pairs->data[box_pairs->len++] = box_pair;
}

double calc_dist(jb box1, jb box2) {
    // we only care about relative distances, so no need for sqrt
    i64 dx = box1.x - box2.x;
    i64 dy = box1.y - box2.y;
    i64 dz = box1.z - box2.z;
    return dx * dx + dy * dy + dz * dz;
}

jbpl gen_jb_pairs(jblist boxes, arena *perm) {
    jbpl box_pairs = {.data = new (perm, jbpair, combinations(boxes.len, 2)),
                      .len = 0};
    for (size b1_idx = 0; b1_idx < boxes.len - 1; b1_idx++) {
        jb box1 = boxes.data[b1_idx];
        for (size b2_idx = b1_idx + 1; b2_idx < boxes.len; b2_idx++) {
            jb box2 = boxes.data[b2_idx];
            i64 dist = calc_dist(box1, box2);
            jbpair pair = {.idx1 = b1_idx, .idx2 = b2_idx, .dist = dist};
            append_jbp(&box_pairs, pair);
        }
    }
    assert(combinations(boxes.len, 2) == box_pairs.len);
    return box_pairs;
}

int comp_jbpairs(const void *pair1_ptr, const void *pair2_ptr) {
    jbpair *pair1 = (jbpair *)pair1_ptr;
    jbpair *pair2 = (jbpair *)pair2_ptr;
    if (pair1->dist < pair2->dist) {
        return -1;
    }
    if (pair2->dist < pair1->dist) {
        return 1;
    }
    return 0;
}

i64 assign_circuits(jbpl box_pairs, jblist boxes, arena *perm) {
    i64list circuit_assignments = {.data = new (perm, i64, boxes.len),
                                   .len = boxes.len};
    i64 max_circuit_id = 1;
    for (usize i = 0; i < box_pairs.len; i++) {
        jbpair box_pair = box_pairs.data[i];
        i64 ca1 = circuit_assignments.data[box_pair.idx1];
        i64 ca2 = circuit_assignments.data[box_pair.idx2];
        i64 circuit_id;
        if (ca1 && ca2) {
            if (ca1 == ca2) { // already in the same circuit nothing to do
                continue;
            }
            // both in existing circuits so join all from the larger cirucuit_id
            // into the lower one
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
        b32 done = 1;
        for (usize f = 0; f < circuit_assignments.len - 1; f++) {
            if (circuit_assignments.data[f] !=
                circuit_assignments.data[f + 1]) {
                done = 0;
                break;
            }
        }
        if (done) {
            return boxes.data[box_pair.idx1].x * boxes.data[box_pair.idx2].x;
        }
    }
    fprintf(stderr, "We reached the end of all circuit pairs but did not get "
                    "them all into a single circuit");
    exit(1);
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 40);
    arena scratch = arena_create(1024L * 4);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    jblist boxes = parse_input(lines, &perm, scratch);
    jbpl box_pairs = gen_jb_pairs(boxes, &perm);
    qsort(box_pairs.data, box_pairs.len, sizeof(jbpair), comp_jbpairs);
    i64 answer = assign_circuits(box_pairs, boxes, &perm);
    printf("Answer: %lld\n", answer);
    return 0;
}
