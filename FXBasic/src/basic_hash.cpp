#include "basic_hash.h"

#include <string.h>

namespace FX {

    static constexpr uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
    static constexpr uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
    static constexpr uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
    static constexpr uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
    static constexpr uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

    static uint64_t read64(const void* p)
    {
        uint64_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    }

    static uint32_t read32(const void* p)
    {
        uint32_t val;
        memcpy(&val, p, sizeof(val));
        return val;
    }

    static uint64_t rotl64(uint64_t val, unsigned char bits)
    {
        return (val << bits) | (val >> (64 - bits));
    }

    static uint64_t round64(uint64_t acc, uint64_t input)
    {
        acc += input * PRIME64_2;
        acc = rotl64(acc, 31);
        acc *= PRIME64_1;
        return acc;
    }

    static uint64_t mergeRound64(uint64_t acc, uint64_t val)
    {
        acc ^= round64(0, val);
        acc *= PRIME64_1;
        acc += PRIME64_4;
        return acc;
    }

    static uint64_t avalanche64(uint64_t hash)
    {
        hash ^= hash >> 33;
        hash *= PRIME64_2;
        hash ^= hash >> 29;
        hash *= PRIME64_3;
        hash ^= hash >> 32;
        return hash;
    }

    uint64_t xxHash64(const void* data, uint64_t length, uint64_t seed)
    {
        const unsigned char* p = (const unsigned char*)data;
        const unsigned char* end = p + length;
        uint64_t hash;

        if (length >= 32)
        {
            const unsigned char* limit = end - 32;

            uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
            uint64_t v2 = seed + PRIME64_2;
            uint64_t v3 = seed;
            uint64_t v4 = seed - PRIME64_1;

            do
            {
                v1 = round64(v1, read64(p)); p += 8;
                v2 = round64(v2, read64(p)); p += 8;
                v3 = round64(v3, read64(p)); p += 8;
                v4 = round64(v4, read64(p)); p += 8;
            } while (p <= limit);

            hash = rotl64(v1, 1) + rotl64(v2, 7) + rotl64(v3, 12) + rotl64(v4, 18);

            hash = mergeRound64(hash, v1);
            hash = mergeRound64(hash, v2);
            hash = mergeRound64(hash, v3);
            hash = mergeRound64(hash, v4);
        }
        else
        {
            hash = seed + PRIME64_5;
        }

        hash += length;

        while (p + 8 <= end)
        {
            uint64_t k1 = round64(0, read64(p));
            hash ^= k1;
            hash = rotl64(hash, 27) * PRIME64_1 + PRIME64_4;
            p += 8;
        }

        if (p + 4 <= end)
        {
            hash ^= (uint64_t)read32(p) * PRIME64_1;
            hash = rotl64(hash, 23) * PRIME64_2 + PRIME64_3;
            p += 4;
        }

        while (p < end)
        {
            hash ^= (uint64_t)(*p) * PRIME64_5;
            hash = rotl64(hash, 11) * PRIME64_1;
            p++;
        }

        return avalanche64(hash);
    }

} // namespace FX
