#include "../util.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    s8 node;
    s8list neighbors;
} adjs;

typedef struct {
    adjs *data;
    size len;
} adjslist;

adjslist new_adjslist(size cap, size len, arena *a) {
    adjslist al = {};
    al.len = len;
    al.data = new (a, adjs, cap);
    return al;
}

void append_adjs(adjslist *al, adjs a) { al->data[al->len++] = a; }

i32 adjscmp(const void *adj1ptr, const void *adj2ptr) {
    adjs adj1 = *(adjs *)adj1ptr;
    adjs adj2 = *(adjs *)adj2ptr;
    return s8cmp(&adj1.node, &adj2.node);
}

adjslist parse_input(s8list lines, arena *perm) {
    adjslist graph = new_adjslist(lines.len, 0, perm);
    for (size i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        s8list line_toks = splitws(line, perm);
        s8 key = line_toks.data[0];
        assert(4 == key.len && ':' == key.data[3]);
        key.len = 3;
        s8list neighbors = slice_s8l(line_toks, 1, line_toks.len);
        adjs edges = {key, neighbors};
        append_adjs(&graph, edges);
    }
    qsort(graph.data, graph.len, sizeof(adjs), adjscmp);
    return graph;
}

s8list adjs_get(adjslist graph, s8 key) {
    i64 start = 0;
    i64 end = graph.len - 1;
    while (start <= end) {
        i64 mid = start + (end - start) / 2;
        adjs el = graph.data[mid];
        i32 cmp = s8cmp(&key, &(el.node));
        if (cmp == 0) {
            return el.neighbors;
        }
        if (cmp < 0) {
            end = mid - 1;
        } else {
            start = mid + 1;
        }
    }
    return (s8list){};
}

s8list dedupe_nodes(s8list nodes, arena *perm) {
    if (nodes.len < 2)
        return nodes;
    s8list deduped = new_s8list(nodes.len, 0, perm);
    qsort(nodes.data, nodes.len, sizeof(s8), s8cmp);
    append_s8(&deduped, nodes.data[0]);
    for (size i = 1; i < nodes.len; i++) {
        s8 cand_node = nodes.data[i];
        s8 latest_node = deduped.data[deduped.len - 1];
        if (!s8equals(cand_node, latest_node)) {
            append_s8(&deduped, cand_node);
        }
    }
    return deduped;
}

s8list get_nodes(adjslist str_graph, arena *perm) {
    size max_num_nodes = str_graph.len;
    // We don't know how many nodes aren't keys. Probably just "out"
    // But idk.  So allocate for keys + neighbors
    for (size i = 0; i <= str_graph.len; i++) {
        s8list neighbors = str_graph.data[i].neighbors;
        max_num_nodes += neighbors.len;
    }
    s8list nodes = new_s8list(max_num_nodes, 0, perm);
    for (size i = 0; i < str_graph.len; i++) {
        adjs node_edges = str_graph.data[i];
        s8 node = node_edges.node;
        s8list neighbors = node_edges.neighbors;
        append_s8(&nodes, node);
        for (size j = 0; j < neighbors.len; j++) {
            s8 neighbor = neighbors.data[j];
            append_s8(&nodes, neighbor);
        }
    }
    return dedupe_nodes(nodes, perm);
}

i64ll make_idx_graph(s8list nodes, adjslist str_graph, arena *perm) {
    i64ll idx_graph = new_i64ll(nodes.len, 0, perm);
    for (size i = 0; i < nodes.len; i++) {
        s8 str_node = nodes.data[i];
        s8list str_neighbors = adjs_get(str_graph, str_node);
        i64list i_neighbors = new_i64list(str_neighbors.len, 0, perm);
        for (size j = 0; j < str_neighbors.len; j++) {
            s8 str_neighbor = str_neighbors.data[j];
            size idx = idx_of_s8l_sorted(nodes, str_neighbor);
            append_i64(&i_neighbors, idx);
        }
        append_i64l(&idx_graph, i_neighbors);
    }
    return idx_graph;
}

i64 dfs(i64ll graph, i64 start, i64 target, i64 friend1, i64 friend2, b32 has_f1,
        b32 has_f2, i64list memo) {
    i64 offset = (has_f1 << 1) | has_f2;
    i64 memoidx = 4 * start + offset;

    if (memo.data[memoidx] != -1) {
        return memo.data[memoidx];
    }

    if (target == start && has_f1 && has_f2) {
        memo.data[memoidx] = 1;
        return 1;
    }

    i64list neighbors = graph.data[start];
    i64 count = 0;
    for (size i = 0; i < neighbors.len; i++) {
        i64 neighbor = neighbors.data[i];
        b32 hasf1 = has_f1 || (friend1 == neighbor);
        b32 hasf2 = has_f2 || (friend2 == neighbor);
        i64 neighbor_res =
            dfs(graph, neighbor, target, friend1, friend2, hasf1, hasf2, memo);
        count += neighbor_res;
    }
    memo.data[memoidx] = count;
    return count;
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 400);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    // adjacency list map of string nodes
    adjslist graph = parse_input(lines, &perm);
    // sorted list of all nodes (not just the ones that are keys)
    s8list nodes = get_nodes(graph, &perm);
    // turn that into a map or node_idx->neighbor_idxs
    i64ll igraph = make_idx_graph(nodes, graph, &perm);
    i64 starti = idx_of_s8l_sorted(nodes, s8("svr"));
    i64 targeti = idx_of_s8l_sorted(nodes, s8("out"));
    // the friends we made along the way
    i64 friend1i = idx_of_s8l_sorted(nodes, s8("dac"));
    i64 friend2i = idx_of_s8l_sorted(nodes, s8("fft"));
    // make a cache f(node_idx, seen_dac, seen_fft)->count
    i64list memo = new_i64list(nodes.len * 4, nodes.len * 4, &perm);
    memset(memo.data, -1, memo.len * sizeof(i64));
    i64 answer = dfs(igraph, starti, targeti, friend1i, friend2i, 0, 0, memo);
    printf("Answer: %lld\n", answer);
    return 0;
}
