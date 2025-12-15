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
    if (nodes.len < 2) return nodes;
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
    // probably overly paranoid but allocate space for nodes to neighbors of every other node
    s8list nodes = new_s8list(str_graph.len * str_graph.len, 0, perm);
    for (size i = 0; i < str_graph.len; i++) {
        adjs node_edges = str_graph.data[i];
        s8 edge_node = node_edges.node;
        s8list neighbors = node_edges.neighbors;
        append_s8(&nodes, edge_node);
        for (size j = 0; j < neighbors.len; j++) {
            s8 neighbor = neighbors.data[j];
            append_s8(&nodes, neighbor);
        }
    }
    return dedupe_nodes(nodes, perm);
}

i64ll make_idx_graph(s8list nodes, adjslist str_graph, arena *perm) {
    i64ll idx_graph = {.len = nodes.len, .data = new (perm, i64list, nodes.len)};
    for (size i = 0; i < nodes.len; i++) {
        s8 str_node = nodes.data[i];
        s8list str_neighbors = adjs_get(str_graph, str_node);
        i64list i_neighbors = {.len = str_neighbors.len,
                               .data = new (perm, i64, str_neighbors.len)};
        for (size j = 0; j < str_neighbors.len; j++) {
            size idx = idx_of_s8l(nodes, str_neighbors.data[j]);
            i_neighbors.data[j] = idx;
        }
        idx_graph.data[i] = i_neighbors;
    }
    return idx_graph;
}

i64 dfs(i64ll igraph, i64 starti, i64 targeti, i64 friend1, i64 friend2, b32 has_f1,
        b32 has_f2, i64list memo) {
    i64 offset = (has_f1 << 1) | has_f2;
    i64 memoidx = 4 * starti + offset;

    if (memo.data[memoidx] != -1) {
        return memo.data[memoidx];
    }

    if (targeti == starti && has_f1 && has_f2) {
        memo.data[memoidx] = 1;
        return 1;
    }

    i64list neighbors = igraph.data[starti];
    i64 count = 0;
    for (size i = 0; i < neighbors.len; i++) {
        i64 neighbor = neighbors.data[i];
        b32 hasf1 = has_f1 || (friend1 == neighbor);
        b32 hasf2 = has_f2 || (friend2 == neighbor);
        i64 neighbor_res =
            dfs(igraph, neighbor, targeti, friend1, friend2, hasf1, hasf2, memo);
        count += neighbor_res;
    }
    memo.data[memoidx] = count;
    return count;
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 40);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    // adjacency list map of string nodes
    adjslist graph = parse_input(lines, &perm);
    // sorted list of all nodes (not just the ones that are keys)
    s8list nodes = get_nodes(graph, &perm);
    // turn that into a map or node_idx->neighbor_idxs
    i64ll igraph = make_idx_graph(nodes, graph, &perm);
    i64 starti = idx_of_s8l(nodes, s8("svr"));
    i64 targeti = idx_of_s8l(nodes, s8("out"));
    i64 friend1i = idx_of_s8l(nodes, s8("dac")); // the friends we made along the way
    i64 friend2i = idx_of_s8l(nodes, s8("fft"));
    // make a cache node_idx->count
    i64list memo = {.len = nodes.len * 4, .data = new (&perm, i64, nodes.len * 4)};
    memset(memo.data, -1, memo.len * sizeof(i64));
    i64 answer = dfs(igraph, starti, targeti, friend1i, friend2i, 0, 0, memo);
    printf("Answer: %lld\n", answer);
    return 0;
}
