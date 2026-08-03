#ifndef _BASIC_HASH_H_
#define _BASIC_HASH_H_

#include <stdint.h>

namespace FX {

    uint64_t xxHash64(const void* pData, unsigned int byteSize, uint64_t seed = 0);

} // namespace FX

#endif // _BASIC_HASH_H_
