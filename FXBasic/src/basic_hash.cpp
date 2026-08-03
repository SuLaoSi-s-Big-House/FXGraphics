#include "basic_hash.h"

#include <string.h>
#include <assert.h>

namespace FX {

    namespace {

        constexpr uint64_t PRIME64_1 = 0x9E3779B185EBCA87ULL;
        constexpr uint64_t PRIME64_2 = 0xC2B2AE3D27D4EB4FULL;
        constexpr uint64_t PRIME64_3 = 0x165667B19E3779F9ULL;
        constexpr uint64_t PRIME64_4 = 0x85EBCA77C2B2AE63ULL;
        constexpr uint64_t PRIME64_5 = 0x27D4EB2F165667C5ULL;

        uint64_t read64(const void* p)
        {
            uint64_t value = 0;
            memcpy(&value, p, sizeof(value));
            return value;
        }

        uint32_t read32(const void* p)
        {
            uint32_t value = 0;
            memcpy(&value, p, sizeof(value));
            return value;
        }

        uint64_t rotl64(uint64_t value, unsigned char bits)
        {
            return (value << bits) | (value >> (64 - bits));
        }

        uint64_t round64(uint64_t acc, uint64_t input)
        {
            acc += input * PRIME64_2;
            acc = rotl64(acc, 31);
            acc *= PRIME64_1;
            return acc;
        }

        uint64_t mergeRound64(uint64_t acc, uint64_t value)
        {
            acc ^= round64(0, value);
            acc *= PRIME64_1;
            acc += PRIME64_4;
            return acc;
        }

        uint64_t avalanche64(uint64_t hash)
        {
            hash ^= hash >> 33;
            hash *= PRIME64_2;
            hash ^= hash >> 29;
            hash *= PRIME64_3;
            hash ^= hash >> 32;
            return hash;
        }

    }  // namespace

    uint64_t xxHash64(const void* pData, unsigned int byteSize, uint64_t seed)
    {
        if (pData == nullptr || byteSize == 0)
        {
            assert(0);
            return 0;
        }

        auto pStart = reinterpret_cast<const uint8_t*>(pData);
        auto pEnd = pStart + byteSize;
        uint64_t hash = 0;

        if (byteSize >= 32)
        {
            auto pLimit = pEnd - 32;

            uint64_t v1 = seed + PRIME64_1 + PRIME64_2;
            uint64_t v2 = seed + PRIME64_2;
            uint64_t v3 = seed;
            uint64_t v4 = seed - PRIME64_1;

            do
            {
                v1 = round64(v1, read64(pStart)); pStart += 8;
                v2 = round64(v2, read64(pStart)); pStart += 8;
                v3 = round64(v3, read64(pStart)); pStart += 8;
                v4 = round64(v4, read64(pStart)); pStart += 8;
            } while (pStart <= pLimit);

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

        hash += byteSize;

        while (pStart + 8 <= pEnd)
        {
            uint64_t k1 = round64(0, read64(pStart));
            hash ^= k1;
            hash = rotl64(hash, 27) * PRIME64_1 + PRIME64_4;
            pStart += 8;
        }

        if (pStart + 4 <= pEnd)
        {
            hash ^= static_cast<uint64_t>(read32(pStart)) * PRIME64_1;
            hash = rotl64(hash, 23) * PRIME64_2 + PRIME64_3;
            pStart += 4;
        }

        while (pStart < pEnd)
        {
            hash ^= static_cast<uint64_t>(*pStart) * PRIME64_5;
            hash = rotl64(hash, 11) * PRIME64_1;
            pStart++;
        }

        return avalanche64(hash);
    }

} // namespace FX
