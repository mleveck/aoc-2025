#ifndef AOC_UTILS_H
#define AOC_UTILS_H

#include <_abort.h>
#include <ctype.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Stealing from https://nullprogram.com/blog/2023/10/08/
typedef uint8_t u8;
typedef int32_t b32;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int64_t i64;
typedef float f32;
typedef double f64;
typedef uintptr_t uptr;
typedef char byte;
typedef ptrdiff_t size;
typedef size_t usize;

// Helper Macros
#define countof(a) (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)

// Strings
#define s8(s)                                                                  \
    (s8) { (u8 *)s, lengthof(s) }
typedef struct s8 {
    u8 *data;
    size len;
} s8;

// Arena
// Stolen from https://nullprogram.com/blog/2023/09/27/
typedef struct arena {
    u8 *beg;
    u8 *end;
} arena;

static inline void *alloc(arena *a, ptrdiff_t size, ptrdiff_t align,
                          ptrdiff_t count) {
    ptrdiff_t padding = -(uintptr_t)a->beg & (align - 1);
    ptrdiff_t available = a->end - a->beg - padding;
    if (available < 0 || count > available / size) {
        abort(); // one possible out-of-memory policy
    }
    void *p = a->beg + padding;
    a->beg += padding + count * size;
    return memset(p, 0, count * size);
}

static inline arena arena_create(size cap) {
    arena a = {0};
    a.beg = (u8 *)malloc(cap);
    a.end = a.beg ? a.beg + cap : 0;
    return a;
}

#define new(a, t, n) (t *)alloc(a, sizeof(t), _Alignof(t), n)

static inline u8 *tocstr(s8 str, arena *a) {
    u8 *cstr = new (a, u8, str.len + 1);
    memcpy(cstr, str.data, str.len);
    cstr[str.len] = '\0';
    return cstr;
}

static inline s8 tos8(char *cstr, arena *a) {
    size len = strlen(cstr);
    u8 *strdata = new (a, u8, len);
    memcpy(strdata, cstr, len);
    return (s8){.data = strdata, .len = len};
}

// File stuff
static inline size fsize(FILE *f) {
    if (0 != fseek(f, 0, SEEK_END)) {
        return -1;
    }
    size len = ftell(f);
    fseek(f, 0, 0);
    return len > 0 ? len : -1;
}

// Todo change fname to s8 and make conversion to cstr helper
static inline s8 slurp(const char *fname, arena *perm) {
    s8 fstr = (s8){.data = NULL, .len = -1};
    FILE *f = fopen(fname, "r");
    size flen = -1;
    u8 *fbuf = NULL;
    // TODO get better error differentiation
    if (!f) {
        puts("couldn't open file");
        goto RET;
    }

    flen = fsize(f);
    if (flen < 0) {
        puts("File size less than 0");
        goto CLEANUP;
    }

    fbuf = new (perm, u8, flen);
    size rsize;
    if ((rsize = fread(fbuf, 1, flen, f)) != flen) {
        // printf("fread was %td\n", rsize);
        fstr = (s8){.data = NULL, .len = -1};
        goto CLEANUP;
    }
    fstr = (s8){.data = fbuf, .len = flen};
CLEANUP:
    fclose(f);
RET:
    return fstr;
}

typedef struct i64list {
    i64 *data;
    size len;
} i64list;

i64 sum(i64list list) {
    i64 res = 0;
    for (usize i = 0; i < list.len; i++) {
        res += list.data[i];
    }
    return res;
}

i64 product(i64list list) {
    i64 res = 1;
    for (usize i = 0; i < list.len; i++) {
        res *= list.data[i];
    }
    return res;
}

typedef struct i64ll {
    i64list *data;
    size len;
} i64ll;

typedef struct s8list {
    s8 *data;
    size len;
} s8list;

typedef struct s8ll {
    s8list *data;
    size len;
} s8ll;

static inline s8list split(s8 text, u8 delim, arena *perm) {
    u8 *start = text.data;
    size num_lines = 0;
    s8list lines = (s8list){.data = NULL, .len = num_lines};
    for (int i = 0; i <= text.len; i++) {
        if (i == text.len || text.data[i] == delim) {
            s8 *str = new (perm, s8, 1);
            str->data = start;
            str->len = &text.data[i] - start;
            start = &text.data[i] + 1;
            if (num_lines == 0) {
                lines.data = str;
            }
            num_lines++;
        }
    }
    lines.len = num_lines;
    // printf("Lines length = %td\n", lines.len);
    return lines;
}

static inline s8list splitws(s8 str, arena *perm) {
    usize start;
    size token_count = 0;
    usize i = 0;
    s8list tokens = (s8list){.data = NULL, .len = token_count};
    while (i < str.len) {
        while (i < str.len && isspace(str.data[i])) {
            i++;
        }
        start = i;
        while (i < str.len && !isspace(str.data[i])) {
            i++;
        }
        usize end = i;
        if (start < end) {
            s8 *token = new (perm, s8, 1);
            token->data = &str.data[start];
            token->len = end - start;
            if (token_count == 0) {
                tokens.data = token;
            }
            token_count++;
        }
    }
    tokens.len = token_count;
    return tokens;
}

s8 stripws(s8 str) {
    size i = 0;
    while (i < str.len && isspace(str.data[i])) {
        i++;
    }

    usize start = i;

    while (i < str.len && !isspace(str.data[i])) {
        i++;
    }
    usize end = i;
    return (s8){.data = &str.data[start], .len = end - start};
}

void reverse_s8(s8 str) {
    size start = 0;
    size end = str.len - 1;
    while (end > start) {
        u8 tmp = str.data[start];
        str.data[start] = str.data[end];
        str.data[end] = tmp;
        start++;
        end--;
    }
}

static inline s8 reversed_s8(s8 str, arena *a) {
    s8 revs8 = {.data = new (a, u8, str.len), .len = str.len};
    size start = 0;
    size end = str.len - 1;
    while (end >= start) {
        revs8.data[start] = str.data[end];
        revs8.data[end] = str.data[start];
        start++;
        end--;
    }
    return revs8;
}

static inline s8list get_lines(s8 text, arena *perm) {
    s8list lines = split(text, '\n', perm);
    // if ends in newline don't have an empty last element
    // matches python splitlines()
    if (lines.data[lines.len - 1].len == 0) {
        lines.len -= 1;
    }
    return lines;
}

static inline s8 slice(s8 str, usize start, usize end) {
    return (s8){
        .data = &str.data[start],
        .len = (size)(str.len <= end ? str.len - start : (size)end - start)};
}

static inline i64 to_long(s8 str, arena scratch) {
    u8 *cstr = tocstr(str, &scratch);
    char *endptr;
    errno = 0;
    i64 num = strtoll((char *)cstr, &endptr, 10);
    if ((char *)cstr == endptr || errno == ERANGE || *endptr != '\0') {
        fprintf(stderr, "Error parsing integer or out of bounds parsing %s.\n",
                cstr);
        exit(1);
    }
    if (*endptr != '\0') {
        fprintf(stderr,
                "Parsed error for %s.  Terminated parsing on unexpected "
                "character %d\n",
                cstr, *endptr);
    }
    return num;
}

typedef struct c64 {
    size r;
    size c;
} c64;

static inline c64 cadd(c64 c1, c64 c2) {
    return (c64){c1.r + c2.r, c1.c + c1.c};
}

typedef struct g64 {
    i64list *data;
    size len;
} g64;

s8 read_input(int argc, char **argv, arena *perm) {
    char *input_fname = argc > 1 ? argv[1] : "sample_input.txt";
    s8 ftext = slurp(input_fname, perm);
    if (ftext.len < 0) {
        fprintf(stderr, "Couldn't open file %s\n", input_fname);
        exit(1);
    }
    return ftext;
}

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

#endif
