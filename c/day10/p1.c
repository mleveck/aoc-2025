#include "../util.h"
#include <assert.h>
#include <limits.h> // For CHAR_BIT and sizeof()
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    u32 lights;
    u32list buttons;
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

void append_u32(u32list *l, u32 num) { l->data[l->len++] = num; }

u32 parse_lights(s8 ls_str) {
    u32 lights = 0;
    s8 light_chars = slice(ls_str, 1, ls_str.len - 1); // strip [ and ]
    for (size i = 0; i < light_chars.len; i++) {
        u8 c = light_chars.data[i];
        switch (c) {
        case '#':
            lights = lights | (1 << ((u32)light_chars.len - (i + 1)));
            break;
        case '.':
            break;
        default:
            fprintf(stderr, "Parse lights err. Unexpected char: %d\n", c);
            exit(1);
        }
    }
    return lights;
}

u32list parse_buttons(s8list bs_strs, u32 lights_len, arena *perm, arena scratch) {
    u32list buttons = new_u32list(bs_strs.len, 0, perm);
    for (size i = 0; i < bs_strs.len; i++) {
        u32 button = 0;
        s8 bstr = bs_strs.data[i];
        bstr = slice(bstr, 1, bstr.len - 1); // strip ( and )
        s8list bstr_toks = split(bstr, ',', &scratch);
        for (size j = 0; j < bstr_toks.len; j++) {
            s8 bstr_tok = bstr_toks.data[j];
            u32 toggle_val = to_u32(bstr_tok, scratch);
            button = button | 1 << (lights_len - (toggle_val + 1));
        }
        append_u32(&buttons, button);
    }
    assert(buttons.len == bs_strs.len);
    return buttons;
}

machinelist parse_input(s8list lines, arena *perm, arena scratch) {
    machinelist ml = new_machinelist(lines.len, 0, perm);
    for (size i = 0; i < lines.len; i++) {
        s8 line = lines.data[i];
        s8list mach_toks = splitws(line, perm);
        s8 lights_str = mach_toks.data[0];
        u32 lights = parse_lights(lights_str);
        u32list buttons = parse_buttons(slice_s8l(mach_toks, 1, mach_toks.len - 1), lights_str.len - 2, perm, scratch);
        append_machine(&ml, (machine){lights, buttons});
    }
    assert(ml.len == lines.len);
    return ml;
}

void print_machine(machine m) {
    printf("Lights: ");
    print_binary(m.lights);
    printf(" ");
    printf("Buttons: ");
    for (size i = 0; i < m.buttons.len; i++) {
        u32 button = m.buttons.data[i];
        print_binary(button);
        printf(", ");
    }
    printf("\n");
}

b32 eval_button_combo(u32 lights, u32list b_combo) {
    u32 lights_state = 0;
    for (size i = 0; i < b_combo.len; i++) {
        u32 button = b_combo.data[i];
        lights_state = lights_state ^ button;
    }
    if (lights_state == lights) {
        return 1;
    }
    return 0;
}

b32 process_button_combos(u32 lights, u32list buttons, u32list bcombo, size bcombo_idx, size r) {
    if (bcombo_idx == bcombo.len) {
        if (eval_button_combo(lights, bcombo)) {
            return 1;
        }
        return 0;
    }
    for (size i = 0; i < buttons.len && buttons.len - i >= r - bcombo_idx; i++) {
        bcombo.data[bcombo_idx] = buttons.data[i];
        u32list buttons_rest = slice_u32l(buttons, i + 1, buttons.len);
        if (process_button_combos(lights, buttons_rest, bcombo, bcombo_idx + 1, r)) {
            return 1;
        }
    }
    return 0;
}

i64 process_all_button_combos(machine m, arena scratch) {
    u32list combo = new_u32list(m.buttons.len, 0, &scratch);
    for (i64 r = 1; r <= m.buttons.len; r++) {
        combo.len = r;
        if (process_button_combos(m.lights, m.buttons, combo, 0, r)) {
            return r;
        }
        memset(combo.data, 0, combo.len * sizeof(u32));
    }
    print_machine(m);
    assert(0 && "Failed to find working button combo");
}

i64 process(machinelist machines, arena scratch) {
    i64 min_button_presses = 0;
    for (size i = 0; i < machines.len; i++) {
        machine m = machines.data[i];
        min_button_presses += process_all_button_combos(m, scratch);
    }
    return min_button_presses;
}

int main(int argc, char **argv) {
    arena perm = arena_create(1024L * 1024 * 40);
    arena scratch = arena_create(1024L * 4);
    s8 input_text = read_input(argc, argv, &perm);

    s8list lines = get_lines(input_text, &perm);
    machinelist machines = parse_input(lines, &perm, scratch);
    i64 answer = process(machines, scratch);
    printf("Answer: %lld\n", answer);
    return 0;
}
