#ifndef COMMON_STRCAT_HPP
#define COMMON_STRCAT_HPP

#include <string>
#include <string_view>

namespace util {

// -----------------------------------------------------------------------------
// StrCat()
// -----------------------------------------------------------------------------
//
// Merges given strings, using no delimiter(s).
//
// `StrCat()` is designed to be the fastest possible way to construct a string
// out of a mix of raw C strings, string_views, strings.

namespace strings_internal {

inline void AppendPieces(std::string *pDest, std::initializer_list<std::string_view> pieces) {
    size_t size = 0;
    for(const auto &rPiece: pieces) {
        size += rPiece.size();
    }
    pDest->reserve(pDest->size() + size);
    for(const auto &rPiece: pieces) {
        pDest->append(rPiece.data(), rPiece.size());
    }
}

inline std::string CatPieces(std::initializer_list<std::string_view> pieces) {
    std::string out;
    AppendPieces(&out, std::move(pieces));
    return out;
}

}// namespace strings_internal

inline std::string StrCat() {
    return std::string();
}

inline std::string StrCat(const std::string_view &rStr) {
    return std::string(rStr.data(), rStr.size());
}

inline std::string StrCat(const std::string_view &rStr1, const std::string_view &rStr2) {
    return strings_internal::CatPieces({rStr1, rStr2});
}

inline std::string StrCat(const std::string_view &rStr1, const std::string_view &rStr2, const std::string_view &rStr3) {
    return strings_internal::CatPieces({rStr1, rStr2, rStr3});
}

// Support 4 or more arguments
template<typename... AV>
inline std::string StrCat(const std::string_view &rStr1, const std::string_view &rStr2, const std::string_view &rStr3, const std::string_view &rStr4, const AV &...args) {
    return strings_internal::CatPieces({rStr1, rStr2, rStr3, rStr4, static_cast<const std::string_view &>(args)...});
}

//-----------------------------------------------

inline void StrAppend(std::string *pDestination, const std::string_view &rStr) {
    pDestination->append(rStr.data(), rStr.size());
}

inline void StrAppend(std::string *pDestination, const std::string_view &rStr1, const std::string_view &rStr2) {
    strings_internal::AppendPieces(pDestination, {rStr1, rStr2});
}

inline void StrAppend(std::string *pDestination, const std::string_view &rStr1, const std::string_view &rStr2, const std::string_view &rStr3) {
    strings_internal::AppendPieces(pDestination, {rStr1, rStr2, rStr3});
}

// Support 4 or more arguments
template<typename... AV>
inline void StrAppend(std::string *pDestination, const std::string_view &rStr1, const std::string_view &rStr2, const std::string_view &rStr3, const std::string_view &rStr4, const AV &...args) {
    strings_internal::AppendPieces(pDestination, {rStr1, rStr2, rStr3, rStr4, static_cast<const std::string_view &>(args)...});
}

}// namespace util

#endif// COMMON_STRCAT_HPP
