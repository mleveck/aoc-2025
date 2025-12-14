#include "../util.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    s8 node;
    s8list neighbors;
} adj;

typedef struct {
    adj* data;
    size len;
} adjlist;

typedef struct {
    size cap;
    size len;
    size head;
    size tail;
    s8* data;
} queue;

queue new_queue(size cap, arena *a) {
    queue q;
    q.len = 0;
    q.head = 0;
    q.tail = 0;
    q.cap = cap;
    q.data = new(a, s8, cap);
    return q;
}

s8 popleft (queue* q) {
    assert(q->len > 0);
    s8 ret = q->data[q->head % q->cap];
    q->len--;
    q->head++; 
    return ret;
}

void push(queue* q, s8 el) {
    assert (q->len < q->cap);
    q->len++;
    q->data[q->tail % q->cap] = el;
    q->tail++;
}

adjlist new_adjlist(size cap, size len, arena *a) {
    adjlist al = {};
    al.len = len;
    al.data = new(a, adj, cap);
    return al;
}

void append_adj(adjlist* al, adj a) {
    al->data[al->len++] = a;
}


i32 adjcmp(const void* adj1ptr, const void* adj2ptr) {
    adj adj1 = *(adj*)adj1ptr;
    adj adj2 = *(adj*)adj2ptr;
    return s8cmp(&adj1.node, &adj2.node);
}


adjlist parse_input(s8list lines, arena* perm){
    adjlist graph = new_adjlist(lines.len, 0, perm);
    for (size i = 0; i < lines.len; i ++) {
        s8 line = lines.data[i];
        s8list line_toks = splitws(line, perm);
        s8 key = line_toks.data[0];
        assert(4 == key.len && ':' == key.data[3]);
        key.len = 3;
        s8list neighbors = slice_s8l(line_toks, 1, line_toks.len);
        adj edges = {key, neighbors};
        append_adj(&graph, edges);
    }
    qsort(graph.data, graph.len, sizeof(adj), adjcmp);
    return graph;
}

s8list adjs_get(adjlist graph, s8 key, arena scratch) {
    i64 start = 0;
    i64 end = graph.len - 1;
    while (start <= end) {
        i64 mid = start + (end - start)/2;
        adj el = graph.data[mid];
        i32 cmp = s8cmp(&key, &(el.node));
        if (cmp == 0){
            return el.neighbors;
        }
        if (cmp < 0) {
            end = mid - 1;
        } else {
            start = mid + 1;
        }
    }
    fprintf(stderr, "Could not find key %s\n", tocstr(key, &scratch));
    exit(1);
}


i64 bfs(adjlist graph, s8 target, arena scratch){
    i64 path_count = 0;
    queue q = new_queue(1000, &scratch);
    s8list seen = new_s8list(10000, 0, &scratch);
    push(&q, s8("you"));
    while(q.len) {
        s8 node = popleft(&q);
        if (in_s8l(seen, node)) continue;
        append_s8(&seen, node);
        if (s8equals(target, node)) {
            path_count ++;
            seen.len = 0;
            continue;
        }
        s8list neighbors = adjs_get(graph,node, scratch);
        for (size i = 0; i < neighbors.len; i ++) {
            push(&q, neighbors.data[i]);
        }
    }
    return path_count;
}

i64 dfs(adjlist graph, s8 target, arena scratch){
    i64 path_count = 0;
    adjlist stack = new_adjlist(100, 0, &scratch);
    append_adj(&stack, (adj){s8("you"), new_s8list(100, 0, &scratch)});
    while(stack.len) {
        adj node_path = stack.data[stack.len - 1];
        stack.len--;
        s8 cand_node = node_path.node;
        s8list path = node_path.neighbors;
        if (s8equals(target, cand_node)){
            path_count++;
            continue;
        }

        s8list neighbors = adjs_get(graph, cand_node, scratch);
        for (size i = 0; i < neighbors.len; i ++){
            s8 neighbor = neighbors.data[i];
            if (in_s8l(path, neighbor)) continue;
            s8list visited = new_s8list(100, 0, &scratch);
            for (size j = 0; j < path.len; j++){
                append_s8(&visited, path.data[j]);
            }
            append_s8(&visited, neighbor);
            append_adj(&stack, (adj){neighbor, visited});
        }
    }
    return path_count;
}


int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 400);
    arena scratch = arena_create(1024L * 1024 * 400);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    adjlist graph = parse_input(lines, &perm);
    adjs_get(graph, s8("you"), scratch);
    i64 answer = dfs(graph, s8("out"), scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
