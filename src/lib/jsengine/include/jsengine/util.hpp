#ifndef ENGINE_UTIL_H
#define ENGINE_UTIL_H

namespace util {
int HasSuffix(const char *ptrStr, const char *ptrSuffix) {
    size_t len = strlen(ptrStr);
    size_t slen = strlen(ptrSuffix);
    return (len >= slen && !memcmp(ptrStr + len - slen, ptrSuffix, slen));
}
}// namespace util

#endif//ENGINE_UTIL_H
