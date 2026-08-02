#ifndef _BASIC_HASH_H_
#define _BASIC_HASH_H_

#include <stdint.h>

namespace FX {

    uint64_t xxHash64(const void* data, uint64_t length, uint64_t seed = 0);

} // namespace FX

#endif // _BASIC_HASH_H_
