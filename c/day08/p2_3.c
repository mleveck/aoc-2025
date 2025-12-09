#include "../util.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**************
 * Claude Code Wrote improvements to my p2.c
 * KD Tree + Proper union find
 * Union Find => almost no perf improvement
 * KD Tree => 76ms ~34 ms 
**************/

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

// K-D tree structures
typedef struct kdnode {
    i64 box_idx;       // Index into original boxes array
    struct kdnode *left;
    struct kdnode *right;
} kdnode;

typedef struct kd_tree {
    kdnode *root;
    blist *boxes;      // Reference to original boxes
} kd_tree;

// Priority queue for k-NN search (max-heap)
typedef struct pq_entry {
    i64 box_idx;
    i64 dist;
} pq_entry;

typedef struct pqueue {
    pq_entry *data;
    size len;
    size cap;
} pqueue;

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

static inline void append_bp(bpl *box_pairs, bpair box_pair) {
    box_pairs->data[box_pairs->len++] = box_pair;
}

static inline i64 calc_dist(b box1, b box2) {
    // we only care about relative distances, so no need for sqrt
    i64 dx = box1.x - box2.x;
    i64 dy = box1.y - box2.y;
    i64 dz = box1.z - box2.z;
    return dx * dx + dy * dy + dz * dz;
}

// Get coordinate by axis (0=x, 1=y, 2=z)
static inline i64 get_coord(b box, int axis) {
    if (axis == 0) return box.x;
    if (axis == 1) return box.y;
    return box.z;
}

// Comparison for qsort by specific axis
typedef struct {
    blist *boxes;
    int axis;
} sort_context;

static sort_context g_sort_ctx;

static int cmp_by_axis(const void *a_ptr, const void *b_ptr) {
    i64 idx_a = *(i64 *)a_ptr;
    i64 idx_b = *(i64 *)b_ptr;
    i64 coord_a = get_coord(g_sort_ctx.boxes->data[idx_a], g_sort_ctx.axis);
    i64 coord_b = get_coord(g_sort_ctx.boxes->data[idx_b], g_sort_ctx.axis);
    if (coord_a < coord_b) return -1;
    if (coord_a > coord_b) return 1;
    return 0;
}

// Build k-d tree recursively
static kdnode *build_kdtree(i64 *indices, size n, int depth, blist *boxes,
                             arena *perm) {
    if (n == 0) return NULL;

    int axis = depth % 3;  // Cycle through x, y, z

    // Sort indices by current axis
    g_sort_ctx.boxes = boxes;
    g_sort_ctx.axis = axis;
    qsort(indices, n, sizeof(i64), cmp_by_axis);

    // Choose median
    size median = n / 2;

    kdnode *node = new (perm, kdnode, 1);
    node->box_idx = indices[median];
    node->left = build_kdtree(indices, median, depth + 1, boxes, perm);
    node->right = build_kdtree(indices + median + 1, n - median - 1,
                                depth + 1, boxes, perm);

    return node;
}

// Priority queue operations (max-heap)
static inline void pq_swap(pqueue *pq, size i, size j) {
    pq_entry tmp = pq->data[i];
    pq->data[i] = pq->data[j];
    pq->data[j] = tmp;
}

static void pq_heapify_down(pqueue *pq, size i) {
    size largest = i;
    size left = 2 * i + 1;
    size right = 2 * i + 2;

    if (left < pq->len && pq->data[left].dist > pq->data[largest].dist)
        largest = left;
    if (right < pq->len && pq->data[right].dist > pq->data[largest].dist)
        largest = right;

    if (largest != i) {
        pq_swap(pq, i, largest);
        pq_heapify_down(pq, largest);
    }
}

static void pq_heapify_up(pqueue *pq, size i) {
    if (i == 0) return;
    size parent = (i - 1) / 2;
    if (pq->data[i].dist > pq->data[parent].dist) {
        pq_swap(pq, i, parent);
        pq_heapify_up(pq, parent);
    }
}

static void pq_insert(pqueue *pq, i64 box_idx, i64 dist) {
    if (pq->len < pq->cap) {
        // Heap not full, just add
        pq->data[pq->len].box_idx = box_idx;
        pq->data[pq->len].dist = dist;
        pq_heapify_up(pq, pq->len);
        pq->len++;
    } else if (dist < pq->data[0].dist) {
        // Replace max if this is smaller
        pq->data[0].box_idx = box_idx;
        pq->data[0].dist = dist;
        pq_heapify_down(pq, 0);
    }
}

static i64 pq_max_dist(pqueue *pq) {
    return pq->len < pq->cap ? INT64_MAX : pq->data[0].dist;
}

// K-NN search in k-d tree
static void knn_search(kdnode *node, b query_point, i64 query_idx, int depth,
                       blist *boxes, pqueue *pq) {
    if (node == NULL) return;

    // Don't include the query point itself
    if (node->box_idx != query_idx) {
        i64 dist = calc_dist(query_point, boxes->data[node->box_idx]);
        pq_insert(pq, node->box_idx, dist);
    }

    int axis = depth % 3;
    i64 query_coord = get_coord(query_point, axis);
    i64 node_coord = get_coord(boxes->data[node->box_idx], axis);
    i64 diff = query_coord - node_coord;

    // Search near side first
    kdnode *near = diff < 0 ? node->left : node->right;
    kdnode *far = diff < 0 ? node->right : node->left;

    knn_search(near, query_point, query_idx, depth + 1, boxes, pq);

    // Check if we need to search far side
    // Only if splitting plane distance < max distance in heap
    if (diff * diff < pq_max_dist(pq)) {
        knn_search(far, query_point, query_idx, depth + 1, boxes, pq);
    }
}

bpl gen_box_pairs_kdtree(blist boxes, i64 target_pairs, arena *perm,
                          arena scratch) {
    // Build k-d tree
    i64 *indices = new (&scratch, i64, boxes.len);
    for (size i = 0; i < boxes.len; i++) {
        indices[i] = i;
    }
    kdnode *root = build_kdtree(indices, boxes.len, 0, &boxes, perm);

    // For each point, find k nearest neighbors
    // For part 2, we need enough pairs to fully connect all boxes
    // With n=1000, worst case needs n-1=999 connections
    i64 k_per_point = 200;  // Query top 200 per point for ~100K total pairs

    // Use a global priority queue to collect all candidate pairs
    size max_pairs = boxes.len * k_per_point;
    bpl box_pairs = {.data = new (perm, bpair, max_pairs), .len = 0};

    pq_entry *pq_data = new (&scratch, pq_entry, k_per_point);

    for (size i = 0; i < boxes.len; i++) {
        pqueue pq = {.data = pq_data, .len = 0, .cap = k_per_point};

        // Find k nearest neighbors for point i
        knn_search(root, boxes.data[i], i, 0, &boxes, &pq);

        // Add all pairs (i, neighbor) where i < neighbor to avoid duplicates
        for (size j = 0; j < pq.len; j++) {
            i64 neighbor_idx = pq.data[j].box_idx;
            if (i < neighbor_idx) {
                bpair pair = {.idx1 = i, .idx2 = neighbor_idx,
                              .dist = pq.data[j].dist};
                append_bp(&box_pairs, pair);
            }
        }
    }

    return box_pairs;
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

// Find with path compression
// Parent array stores: negative values at roots (=-size), positive values = parent index
static inline i64 find_root(i64list *parent, i64 i) {
    if (parent->data[i] < 0) {
        return i;  // This is a root (negative = -size)
    }
    // Path compression: update parent to point directly to root
    return parent->data[i] = find_root(parent, parent->data[i]);
}

// Union by size: merge smaller set into larger
static inline void union_sets(i64list *parent, i64 i, i64 j) {
    i64 root_i = find_root(parent, i);
    i64 root_j = find_root(parent, j);

    if (root_i == root_j) {
        return;  // Already in same set
    }

    // Union by size: root stores negative size
    // More negative = larger set
    if (parent->data[root_i] < parent->data[root_j]) {
        // root_i is larger (more negative)
        parent->data[root_i] += parent->data[root_j];  // Merge sizes
        parent->data[root_j] = root_i;  // Point j's root to i
    } else {
        // root_j is larger or equal
        parent->data[root_j] += parent->data[root_i];
        parent->data[root_i] = root_j;
    }
}

// Part 2: Connect boxes until all are unified into one circuit
// Returns product of X coordinates of the last pair that unifies everything
i64 assign_circuits_until_unified(bpl box_pairs, blist boxes, arena *perm) {
    // Initialize union-find: each box starts as its own component
    i64list parent = {.data = new (perm, i64, boxes.len), .len = boxes.len};
    for (usize i = 0; i < boxes.len; i++) {
        parent.data[i] = -1;  // -1 means root with size 1
    }

    // Track number of separate components
    i64 num_components = boxes.len;

    // Process pairs in order of increasing distance
    for (usize i = 0; i < box_pairs.len; i++) {
        bpair pair = box_pairs.data[i];

        i64 root1 = find_root(&parent, pair.idx1);
        i64 root2 = find_root(&parent, pair.idx2);

        if (root1 != root2) {
            // Different components - union them
            union_sets(&parent, pair.idx1, pair.idx2);
            num_components--;  // Merged two components into one

            if (num_components == 1) {
                // All boxes now in single circuit!
                // Return product of X coordinates
                return boxes.data[pair.idx1].x * boxes.data[pair.idx2].x;
            }
        }
        // If root1 == root2, they're already connected, skip
    }

    // Should never reach here if we have enough pairs
    fprintf(stderr, "Error: Ran out of pairs before unifying all components\n");
    fprintf(stderr, "Remaining components: %lld\n", num_components);
    exit(1);
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 40);
    arena scratch = arena_create(1024L * 1024 * 10);  // Increased for k-d tree
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    blist boxes = parse_input(lines, &perm, scratch);

    // Use k-d tree to generate candidate pairs
    // We don't know how many connections needed, but k=200 per point gives ~100K pairs
    bpl box_pairs = gen_box_pairs_kdtree(boxes, 0, &perm, scratch);

    // Sort by distance (closest first)
    qsort(box_pairs.data, box_pairs.len, sizeof(bpair), cmp_bpairs);

    // Connect until all unified, return answer directly
    i64 answer = assign_circuits_until_unified(box_pairs, boxes, &perm);
    printf("Answer: %lld\n", answer);
    return 0;
}
