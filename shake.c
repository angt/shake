#include "shake.h"

#include <inttypes.h>

static const uint64_t ct[24] = {
    UINT64_C(0x0000000000000001), UINT64_C(0x0000000000008082),
    UINT64_C(0x800000000000808a), UINT64_C(0x8000000080008000),
    UINT64_C(0x000000000000808b), UINT64_C(0x0000000080000001),
    UINT64_C(0x8000000080008081), UINT64_C(0x8000000000008009),
    UINT64_C(0x000000000000008a), UINT64_C(0x0000000000000088),
    UINT64_C(0x0000000080008009), UINT64_C(0x000000008000000a),
    UINT64_C(0x000000008000808b), UINT64_C(0x800000000000008b),
    UINT64_C(0x8000000000008089), UINT64_C(0x8000000000008003),
    UINT64_C(0x8000000000008002), UINT64_C(0x8000000000000080),
    UINT64_C(0x000000000000800a), UINT64_C(0x800000008000000a),
    UINT64_C(0x8000000080008081), UINT64_C(0x8000000000008080),
    UINT64_C(0x0000000080000001), UINT64_C(0x8000000080008008),
};

static const unsigned rho[24] = {
     1,  3,  6, 10, 15, 21, 28, 36, 45, 55,  2, 14,
    27, 41, 56,  8, 25, 43, 62, 18, 39, 61, 20, 44,
};

static const unsigned pi[24] = {
    10,  7, 11, 17, 18, 3,  5, 16,  8, 21, 24, 4,
    15, 23, 19, 13, 12, 2, 20, 14, 22,  9,  6, 1,
};

static inline uint64_t
rotl64(uint64_t x, uint64_t y) {
    return (x << y) | (x >> (64U - y));
}

static inline void
inject(uint64_t *const dst, const unsigned p, const uint64_t val)
{
    dst[p >> 3] ^= val << ((p & 7) << 3);
}

static inline uint8_t
extract(const uint64_t *const src, const unsigned p)
{
    return src[p >> 3] >> ((p & 7) << 3);
}

static void
keccakf(uint64_t s[25])
{
    uint64_t bc[5];

    for (int k = 0; k < 24; k++) {
        for (int i = 0; i < 5; i++)
            bc[i] = s[i] ^ s[i + 5] ^ s[i + 10] ^ s[i + 15] ^ s[i + 20];

        for (int i = 0; i < 5; i++) {
            const uint64_t t = bc[(i + 4) % 5] ^ rotl64(bc[(i + 1) % 5], 1);
            for (int j = 0; j < 25; j += 5)
                s[j + i] ^= t;
        }
        uint64_t t = s[1];

        for (int i = 0; i < 24; i++) {
            int j = pi[i];
            bc[0] = s[j];
            s[j] = rotl64(t, rho[i]);
            t = bc[0];
        }
        for (int j = 0; j < 25; j += 5) {
            for (int i = 0; i < 5; i++)
                bc[i] = s[j + i];
            for (int i = 0; i < 5; i++)
                s[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
        }
        s[0] ^= ct[k];
    }
}

void
shake_init(struct shake *const shake,
           const unsigned bits)
{
    *shake = (struct shake){.rate = 200 - (bits >> 2)};
}

void
shake_inject(struct shake *const shake,
             const void *const src, const size_t size)
{
    for (size_t i = 0; i < size; i++) {
        inject(shake->state, shake->pos, ((uint8_t *)src)[i]);
        shake->pos++;

        if (shake->pos == shake->rate) {
            keccakf(shake->state);
            shake->pos = 0;
        }
    }
}

void
shake_flip(struct shake *const shake)
{
    inject(shake->state, shake->pos, 0x1F);
    inject(shake->state, shake->rate - 1, 0x80);
    shake->pos = shake->rate;
}

void
shake_extract(struct shake *const shake,
              void *const dst, const size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (shake->pos == shake->rate) {
            keccakf(shake->state);
            shake->pos = 0;
        }
        ((uint8_t *)dst)[i] = extract(shake->state, shake->pos);
        shake->pos++;
    }
}

#ifdef SHAKE_TEST
#include <stdio.h>
#include <time.h>
#include <string.h>

int
main(int argc, char **argv)
{
    struct shake shake;
    uint8_t hash[32];
    unsigned bits = 128;
    char *data = argv[1] ? argv[1] : "test";

    clock_t t0 = clock();

    for (int i = 0; i < 10000; i++) {
        shake_init(&shake, bits);
        shake_inject(&shake, data, strlen(data));
        shake_flip(&shake);
        shake_extract(&shake, hash, sizeof(hash));
    }
    clock_t t1 = clock();

    printf("time: %.06fs\n", (t1 - t0) / (CLOCKS_PER_SEC * 10000.0));
    printf("shake%u(\"%s\"): ", bits, data);

    for (int i = 0; i < 32; i++)
        printf("%02" PRIx8, hash[i]);
    printf("\n");

    return 0;
}
#endif
