#include "../util.h"
#include <assert.h>
#include <glpk.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    i32 *data;
    size len;
} i32list;

typedef struct {
    f64 *data;
    size len;
} f64list;

typedef struct {
    i32list *data;
    size len;
} i32ll;

i32list new_i32list(size cap, size len, arena *a) {
    i32list l = {0};
    l.len = len;
    l.data = new (a, i32, cap);
    return l;
}

f64list new_f64list(size cap, size len, arena *a) {
    f64list l = {0};
    l.len = len;
    l.data = new (a, f64, cap);
    return l;
}

void append_f64(f64list *l, f64 num) { l->data[l->len++] = num; }

void append_i32(i32list *l, i32 num) { l->data[l->len++] = num; }

i32ll new_i32ll(size cap, size len, arena *a) {
    i32ll ll = {0};
    ll.len = len;
    ll.data = new (a, i32list, cap);
    return ll;
}

void append_i32list(i32ll *ll, i32list l) { ll->data[ll->len++] = l; }

typedef struct {
    i32list counters;
    i32ll buttons;
} machine;

typedef struct {
    machine *data;
    size len;
} machinelist;

machinelist new_machinelist(size cap, size len, arena *a) {
    machinelist ml;
    ml.len = len;
    ml.data = new (a, machine, cap);
    return ml;
}

void append_machine(machinelist *ms, machine m) { ms->data[ms->len++] = m; }

i32list parse_counters(s8 counters_str, arena *perm, arena scratch) {
    s8 counters_str_s = slice(counters_str, 1, counters_str.len - 1); // strip { and }
    s8list counters_toks = split(counters_str_s, ',', &scratch);
    i32list counters = new_i32list(counters_toks.len, 0, perm);
    for (size i = 0; i < counters_toks.len; i++) {
        s8 counter_tok = counters_toks.data[i];
        i32 counter = to_i32(counter_tok, scratch);
        append_i32(&counters, counter);
    }
    return counters;
}

i32ll parse_buttons(s8list buttons_strs, arena *perm, arena scratch) {
    i32ll buttons = new_i32ll(buttons_strs.len, 0, perm);
    for (size i = 0; i < buttons_strs.len; i++) {
        s8 bstr = buttons_strs.data[i];
        bstr = slice(bstr, 1, bstr.len - 1); // strip ( and )
        s8list bstr_toks = split(bstr, ',', &scratch);
        i32list counter_idxs = new_i32list(bstr_toks.len, 0, perm);
        for (size j = 0; j < bstr_toks.len; j++) {
            s8 bstr_tok = bstr_toks.data[j];
            i32 counter_idx = (i32)to_i32(bstr_tok, scratch);
            append_i32(&counter_idxs, counter_idx);
        }
        append_i32list(&buttons, counter_idxs);
    }
    assert(buttons.len == buttons_strs.len);
    return buttons;
}

machinelist parse_input(s8list lines, arena *perm, arena scratch) {
    machinelist ml = new_machinelist(lines.len, 0, perm);
    for (size i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        s8list mach_toks = splitws(line, perm);
        s8 counters_str = mach_toks.data[mach_toks.len - 1];
        s8list buttons_strs = slice_s8l(mach_toks, 1, mach_toks.len - 1);
        i32ll buttons = parse_buttons(buttons_strs, perm, scratch);
        i32list counters = parse_counters(counters_str, perm, scratch);
        append_machine(&ml, (machine){counters, buttons});
    }
    assert(ml.len == lines.len);
    return ml;
}

void print_machine(machine m) {
    printf("Counters: ");
    for (size j = 0; j < m.counters.len; j++) {
        printf("%d ", m.counters.data[j]);
    }
    printf(" | ");
    printf("Buttons: ");
    for (size i = 0; i < m.buttons.len; i++) {
        i32list button = m.buttons.data[i];
        printf(" (");
        for (size k = 0; k < button.len; k++) {
            printf("%d", button.data[k]);
            printf(" ");
        }
        printf(") ");
    }
    printf("\n");
}

i64 process_machine(machine m, arena scratch) {
    glp_prob *lp = glp_create_prob();
    glp_set_obj_dir(lp, GLP_MIN);
    i32ll buttons = m.buttons;
    i32list counters = m.counters;

    glp_add_cols(lp, buttons.len);
    for (size i = 1; i <= buttons.len; i++) {
        glp_set_col_kind(lp, i, GLP_IV); // button presses are int vars
        glp_set_col_bnds(lp, i, GLP_LO, 0.0, 0.0); // no negative presses
        glp_set_obj_coef(lp, i, 1.0); // press a button inc count 1
    }

    glp_add_rows(lp, counters.len);
    for (size i = 1; i <= counters.len; i++) {
        i32 counter = counters.data[i - 1];
        // have to hit the exact counter values
        glp_set_row_bnds(lp, i, GLP_FX, (double)counter, (double)counter);
    }

    size num_vals = 0;
    for (size i = 0; i < buttons.len; i++) {
        i32list button = buttons.data[i];
        num_vals += button.len;
    }

    i32list r_idxs = new_i32list(num_vals + 1, 0, &scratch);
    i32list c_idxs = new_i32list(num_vals + 1, 0, &scratch);
    f64list vals = new_f64list(num_vals + 1, 0, &scratch);

    append_i32(&r_idxs, 0); // glpk use 1 indexing and needs dummy vals at start of array
    append_i32(&c_idxs, 0);
    append_f64(&vals, 0.0);

    // all the non-zero values in the sparse matrix are 1
    for (size i = 0; i < num_vals; i++) {
        append_f64(&vals, 1.0);
    }

    // Each button is a column in the input matrix
    // It's values are the row indexes to set
    // Again, 1 based indexing
    for (i32 c = 1; c <= buttons.len; c++) {
        i32list button = buttons.data[c - 1];
        for (size i = 0; i < button.len; i++) {
            i32 r = button.data[i] + 1;
            append_i32(&c_idxs, c);
            append_i32(&r_idxs, r);
        }
    }

    glp_load_matrix(lp, num_vals, r_idxs.data, c_idxs.data, vals.data);

    // settings to quiet glpk down
    glp_smcp parm_simplex;
    glp_init_smcp(&parm_simplex);
    parm_simplex.msg_lev = GLP_MSG_OFF;

    // solve the relaxed problem, could be floating point
    glp_simplex(lp, &parm_simplex);

    // settings to quiet glpk down
    glp_iocp parm_intopt;
    glp_init_iocp(&parm_intopt);
    parm_intopt.msg_lev = GLP_MSG_OFF;

    // find the integer solutions
    glp_intopt(lp, &parm_intopt);

    // error if we didn't find a optimal solution
    int status = glp_mip_status(lp);
    assert(status == GLP_OPT);

    f64 min_button_presses = glp_mip_obj_val(lp);
    glp_delete_prob(lp);
    return min_button_presses;
}

f64 process(machinelist ml, arena scratch) {
    f64 button_presses = 0.0;
    for (size i = 0; i < ml.len; i++) {
        machine m = ml.data[i];
        button_presses += process_machine(m, scratch);
    }
    return button_presses;
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 400);
    arena scratch = arena_create(1024L * 40);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    machinelist machines = parse_input(lines, &perm, scratch);
    for (size i = 0; i < machines.len; i++) {
        machine m = machines.data[i];
        print_machine(m);
    }
    f64 answer = process(machines, scratch);
    printf("Answer: %f\n", answer);
    return 0;
}
