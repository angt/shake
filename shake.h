#pragma once

#include <stddef.h>
#include <stdint.h>

struct shake {
    uint64_t state[25];
    unsigned rate;
    unsigned pos;
};

void
shake_init(struct shake *const shake,
           const unsigned bits);

void
shake_inject(struct shake *const shake,
             const void *const src,
             const size_t size);

void
shake_flip(struct shake *const shake);

void
shake_extract(struct shake *const shake,
              void *const dst,
              const size_t size);
