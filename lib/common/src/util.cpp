module;

#include <string>

module shadow.util;

namespace shadow::util {
bool HasSuffix(const char *ptrStr, const char *ptrSuffix) {
    const size_t len = strlen(ptrStr);
    const size_t slen = strlen(ptrSuffix);
    return (len >= slen and !memcmp(ptrStr + len - slen, ptrSuffix, slen));
}
}// namespace shadow::util

// module shadow.util;
// module;
