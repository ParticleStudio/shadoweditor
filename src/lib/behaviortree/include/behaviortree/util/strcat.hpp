#ifndef BEHAVIORTREE_STRCAT_HPP
#define BEHAVIORTREE_STRCAT_HPP

#include <string>
#include <string_view>

namespace behaviortree {

// -----------------------------------------------------------------------------
// StrCat()
// -----------------------------------------------------------------------------
//
// Merges given strings, using no delimiter(s).
//
// `StrCat()` is designed to be the fastest possible way to construct a string
// out of a mix of raw C strings, string_views, strings.

namespace strings_internal {

inline void AppendPieces(
        std::string *ptrDest, std::initializer_list<std::string_view> pieces
) {
    size_t size = 0;
    for(const auto &refPiece: pieces) {
        size += refPiece.size();
    }
    ptrDest->reserve(ptrDest->size() + size);
    for(const auto &refPiece: pieces) {
        ptrDest->append(refPiece.data(), refPiece.size());
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

inline std::string StrCat(const std::string_view &refStr) {
    return std::string(refStr.data(), refStr.size());
}

inline std::string StrCat(
        const std::string_view &refStr1, const std::string_view &refStr2
) {
    return strings_internal::CatPieces({refStr1, refStr2});
}

inline std::string StrCat(
        const std::string_view &refStr1, const std::string_view &refStr2,
        const std::string_view &refStr3
) {
    return strings_internal::CatPieces({refStr1, refStr2, refStr3});
}

// Support 4 or more arguments
template<typename... AV>
inline std::string StrCat(
        const std::string_view &refStr1, const std::string_view &refStr2,
        const std::string_view &refStr3, const std::string_view &refStr4,
        const AV &...args
) {
    return strings_internal::CatPieces(
            {refStr1, refStr2, refStr3, refStr4,
             static_cast<const std::string_view &>(args)...}
    );
}

//-----------------------------------------------

inline void StrAppend(
        std::string *ptrDestination, const std::string_view &refStr
) {
    ptrDestination->append(refStr.data(), refStr.size());
}

inline void StrAppend(
        std::string *ptrDestination, const std::string_view &refStr1,
        const std::string_view &refStr2
) {
    strings_internal::AppendPieces(ptrDestination, {refStr1, refStr2});
}

inline void StrAppend(
        std::string *ptrDestination, const std::string_view &refStr1,
        const std::string_view &refStr2, const std::string_view &refStr3
) {
    strings_internal::AppendPieces(ptrDestination, {refStr1, refStr2, refStr3});
}

// Support 4 or more arguments
template<typename... AV>
inline void StrAppend(
        std::string *ptrDestination, const std::string_view &refStr1,
        const std::string_view &refStr2, const std::string_view &refStr3,
        const std::string_view &refStr4, const AV &...args
) {
    strings_internal::AppendPieces(
            ptrDestination, {refStr1, refStr2, refStr3, refStr4,
                             static_cast<const std::string_view &>(args)...}
    );
}

}// namespace behaviortree

#endif// BEHAVIORTREE_STRCAT_HPP
