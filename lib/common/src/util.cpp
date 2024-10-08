module;

module common.util;

import <cstdlib>;
import <string>;

namespace util {
int HasSuffix(const char *ptrStr, const char *ptrSuffix) {
    size_t len = strlen(ptrStr);
    size_t slen = strlen(ptrSuffix);
    return (len >= slen and !memcmp(ptrStr + len - slen, ptrSuffix, slen));
}
}// namespace util

// module common.util;
// module;
