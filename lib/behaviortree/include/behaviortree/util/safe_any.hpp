#ifndef BEHAVIORTREE_SAFE_ANY_HPP
#define BEHAVIORTREE_SAFE_ANY_HPP

#if __has_include(<charconv>)
#    include <charconv>
#endif

#include <string>
#include <type_traits>
#include <typeindex>

#include "behaviortree/contrib/any.hpp"
#include "behaviortree/contrib/expected.hpp"
#include "behaviortree/util/convert_impl.hpp"
#include "behaviortree/util/demangle_util.h"
#include "common/strcat.hpp"

namespace behaviortree {

static std::type_index UndefinedAnyType = typeid(nullptr);

// Rational: since type erased numbers will always use at least 8 bytes
// it is faster to cast everything to either double, uint64_t or int64_t.
class Any {
    template<typename T>
    using EnableIntegral = typename std::enable_if<
            std::is_integral<T>::value || std::is_enum<T>::value>::type *;

    template<typename T>
    using EnableNonIntegral = typename std::enable_if<
            !std::is_integral<T>::value && !std::is_enum<T>::value>::type *;

    template<typename T>
    using EnableString =
            typename std::enable_if<std::is_same<T, std::string>::value>::type *;

    template<typename T>
    using EnableArithmetic =
            typename std::enable_if<std::is_arithmetic<T>::value>::type *;

    template<typename T>
    using EnableEnum = typename std::enable_if<std::is_enum<T>::value>::type *;

    template<typename T>
    using EnableUnknownType = typename std::enable_if<
            !std::is_arithmetic<T>::value && !std::is_enum<T>::value &&
            !std::is_same<T, std::string>::value>::type *;

    template<typename T>
    nonstd::expected<T, std::string> StringToNumber() const;

 public:
    Any(): m_OriginalType(UndefinedAnyType) {}

    ~Any() = default;

    Any(const Any &rOther): m_Any(rOther.m_Any),
                            m_OriginalType(rOther.m_OriginalType) {}

    Any(Any &&rOther): m_Any(std::move(rOther.m_Any)),
                       m_OriginalType(rOther.m_OriginalType) {}

    explicit Any(const double &rValue): m_Any(rValue),
                                        m_OriginalType(typeid(double)) {}

    explicit Any(const uint64_t &rValue): m_Any(rValue),
                                          m_OriginalType(typeid(uint64_t)) {}

    explicit Any(const float &rValue): m_Any(double(rValue)),
                                       m_OriginalType(typeid(float)) {}

    explicit Any(const std::string &rStr): m_Any(SafeAny::SimpleString(rStr)),
                                           m_OriginalType(typeid(std::string)) {}

    explicit Any(const char *pStr): m_Any(SafeAny::SimpleString(pStr)),
                                    m_OriginalType(typeid(std::string)) {}

    explicit Any(const SafeAny::SimpleString &rStr): m_Any(rStr),
                                                     m_OriginalType(typeid(std::string)) {}

    explicit Any(const std::string_view &rStr): m_Any(SafeAny::SimpleString(rStr)),
                                                m_OriginalType(typeid(std::string)) {}

    // all the other integrals are casted to int64_t
    template<typename T>
    explicit Any(const T &rValue, EnableIntegral<T> = 0): m_Any(int64_t(rValue)),
                                                          m_OriginalType(typeid(T)) {}

    explicit Any(const std::type_index &rType): m_OriginalType(rType) {}

    // default for other custom types
    template<typename T>
    explicit Any(const T &rValue, EnableNonIntegral<T> = 0): m_Any(rValue),
                                                             m_OriginalType(typeid(T)) {
        static_assert(!std::is_reference<T>::value, "Any can not contain references");
    }

    Any &operator=(const Any &rOther);

    [[nodiscard]] bool IsNumber() const;

    [[nodiscard]] bool IsIntegral() const;

    [[nodiscard]] bool IsString() const {
        return m_Any.Type() == typeid(SafeAny::SimpleString);
    }

    // check is the original type is equal to T
    template<typename T>
    [[nodiscard]] bool IsType() const {
        return m_OriginalType == typeid(T);
    }

    // copy the value (casting into dst). We preserve the destination type.
    void CopyInto(Any &rDst);

    // this is different from any_cast, because if allows safe
    // conversions between arithmetic values and from/to string.
    template<typename T>
    nonstd::expected<T, std::string> TryCast() const;

    // same as tryCast, but throws if fails
    template<typename T>
    [[nodiscard]] T Cast() const {
        if(auto res = TryCast<T>()) {
            return res.value();
        } else {
            throw std::runtime_error(res.error());
        }
    }

    // Method to access the value by pointer.
    // It will return nullptr, if the user try to cast it to a
    // wrong type or if Any was empty.
    template<typename T>
    [[nodiscard]] T *CastPtr() {
        static_assert(!std::is_same_v<T, float>, "The value has been casted internally to [double]. Use that instead");
        static_assert(!SafeAny::details::is_integer<T>() || std::is_same_v<T, uint64_t>, "The value has been casted internally to [int64_t]. Use that instead");

        return m_Any.Empty() ? nullptr : linb::any_cast<T>(&m_Any);
    }

    // This is the original type
    [[nodiscard]] const std::type_index &Type() const noexcept {
        return m_OriginalType;
    }

    // This is the type we casted to, internally
    [[nodiscard]] const std::type_info &CastedType() const noexcept {
        return m_Any.Type();
    }

    [[nodiscard]] bool Empty() const noexcept {
        return m_Any.Empty();
    }

 private:
    linb::any m_Any;
    std::type_index m_OriginalType;

    //----------------------------

    template<typename DST>
    nonstd::expected<DST, std::string> Convert(EnableString<DST> = 0) const;

    template<typename DST>
    nonstd::expected<DST, std::string> Convert(EnableArithmetic<DST> = nullptr)
            const;

    template<typename DST>
    nonstd::expected<DST, std::string> Convert(EnableEnum<DST> = 0) const;

    template<typename DST>
    nonstd::expected<DST, std::string> Convert(EnableUnknownType<DST> = 0)
            const {
        return nonstd::make_unexpected(errorMsg<DST>());
    }

    template<typename T>
    std::string errorMsg() const {
        return util::StrCat("[Any::Convert]: no known safe conversion between [", Demangle(Type()), "] and [", Demangle(typeid(T)), "]");
    }
};

//-------------------------------------------------------------
//-------------------------------------------------------------
//-------------------------------------------------------------

template<typename SRC, typename TO>
inline bool ValidCast(const SRC &rVal) {
    return (rVal == static_cast<SRC>(static_cast<TO>(rVal)));
}

template<typename T>
inline bool IsCastingSafe(const std::type_index &rType, const T &rVal) {
    if(rType == typeid(T)) {
        return true;
    }

    if(std::type_index(typeid(uint8_t)) == rType) {
        return ValidCast<T, uint8_t>(rVal);
    }
    if(std::type_index(typeid(uint16_t)) == rType) {
        return ValidCast<T, uint16_t>(rVal);
    }
    if(std::type_index(typeid(uint32_t)) == rType) {
        return ValidCast<T, uint32_t>(rVal);
    }
    if(std::type_index(typeid(uint64_t)) == rType) {
        return ValidCast<T, uint64_t>(rVal);
    }
    //------------
    if(std::type_index(typeid(int8_t)) == rType) {
        return ValidCast<T, int8_t>(rVal);
    }
    if(std::type_index(typeid(int16_t)) == rType) {
        return ValidCast<T, int16_t>(rVal);
    }
    if(std::type_index(typeid(int32_t)) == rType) {
        return ValidCast<T, int32_t>(rVal);
    }
    if(std::type_index(typeid(int64_t)) == rType) {
        return ValidCast<T, int64_t>(rVal);
    }
    //------------
    if(std::type_index(typeid(float)) == rType) {
        return ValidCast<T, float>(rVal);
    }
    if(std::type_index(typeid(double)) == rType) {
        return ValidCast<T, double>(rVal);
    }
    return false;
}

inline Any &Any::operator=(const Any &rOther) {
    this->m_Any = rOther.m_Any;
    this->m_OriginalType = rOther.m_OriginalType;
    return *this;
}

inline bool Any::IsNumber() const {
    return m_Any.Type() == typeid(int64_t) ||
           m_Any.Type() == typeid(uint64_t) || m_Any.Type() == typeid(double);
}

inline bool Any::IsIntegral() const {
    return m_Any.Type() == typeid(int64_t) || m_Any.Type() == typeid(uint64_t);
}

inline void Any::CopyInto(Any &rDst) {
    if(rDst.Empty()) {
        rDst = *this;
        return;
    }

    const auto &refDstType = rDst.CastedType();

    if((CastedType() == refDstType) || (IsString() && rDst.IsString())) {
        rDst.m_Any = m_Any;
    } else if(IsNumber() && rDst.IsNumber()) {
        if(refDstType == typeid(int64_t)) {
            rDst.m_Any = Cast<int64_t>();
        } else if(refDstType == typeid(uint64_t)) {
            rDst.m_Any = Cast<uint64_t>();
        } else if(refDstType == typeid(double)) {
            rDst.m_Any = Cast<double>();
        } else {
            throw std::runtime_error("Any::CopyInto fails");
        }
    } else {
        throw std::runtime_error("Any::CopyInto fails");
    }
}

template<typename DST>
inline nonstd::expected<DST, std::string> Any::Convert(EnableString<DST>) const {
    const auto &rType = m_Any.Type();

    if(rType == typeid(SafeAny::SimpleString)) {
        return linb::any_cast<SafeAny::SimpleString>(m_Any).toStdString();
    } else if(rType == typeid(int64_t)) {
        return std::to_string(linb::any_cast<int64_t>(m_Any));
    } else if(rType == typeid(uint64_t)) {
        return std::to_string(linb::any_cast<uint64_t>(m_Any));
    } else if(rType == typeid(double)) {
        return std::to_string(linb::any_cast<double>(m_Any));
    }

    return nonstd::make_unexpected(errorMsg<DST>());
}

template<typename T>
inline nonstd::expected<T, std::string> Any::StringToNumber() const {
    static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, "Expecting a numeric Type");

    const auto str = linb::any_cast<SafeAny::SimpleString>(m_Any);
#if __cpp_lib_to_chars >= 201611L
    T out;
    auto [ptr, err] = std::from_chars(str.data(), str.data() + str.size(), out);
    if(err == std::errc()) {
        return out;
    } else {
        return nonstd::make_unexpected("Any failed string to number conversion"
        );
    }
#else
    try {
        if constexpr(std::is_same_v<T, uint16_t>) {
            return std::stoul(str.toStdString());
        }
        if constexpr(std::is_integral_v<T>) {
            const int64_t val = std::stol(str.toStdString());
            Any temp_any(val);
            return temp_any.convert<T>();
        }
        if constexpr(std::is_floating_point_v<T>) {
            return std::stod(str.toStdString());
        }
    } catch(...) {
        return nonstd::make_unexpected("Any failed string to number conversion"
        );
    }
#endif
    return nonstd::make_unexpected("Any conversion from string failed");
}

template<typename DST>
inline nonstd::expected<DST, std::string> Any::Convert(EnableEnum<DST>) const {
    using SafeAny::details::convertNumber;

    const auto &refType = m_Any.Type();

    if(refType == typeid(int64_t)) {
        auto out = linb::any_cast<int64_t>(m_Any);
        return static_cast<DST>(out);
    } else if(refType == typeid(uint64_t)) {
        auto out = linb::any_cast<uint64_t>(m_Any);
        return static_cast<DST>(out);
    }

    return nonstd::make_unexpected(errorMsg<DST>());
}

template<typename DST>
inline nonstd::expected<DST, std::string> Any::Convert(EnableArithmetic<DST>)
        const {
    using SafeAny::details::convertNumber;
    DST out;

    const auto &refType = m_Any.Type();

    if(refType == typeid(int64_t)) {
        convertNumber<int64_t, DST>(linb::any_cast<int64_t>(m_Any), out);
    } else if(refType == typeid(uint64_t)) {
        convertNumber<uint64_t, DST>(linb::any_cast<uint64_t>(m_Any), out);
    } else if(refType == typeid(double)) {
        convertNumber<double, DST>(linb::any_cast<double>(m_Any), out);
    } else {
        return nonstd::make_unexpected(errorMsg<DST>());
    }
    return out;
}

template<typename T>
inline nonstd::expected<T, std::string> Any::TryCast() const {
    static_assert(!std::is_reference<T>::value, "Any::Cast uses value semantic, can not Cast to reference");

    if(m_Any.Empty()) {
        throw std::runtime_error("Any::Cast failed because it is Empty");
    }

    if(CastedType() == typeid(T)) {
        return linb::any_cast<T>(m_Any);
    }

    // special case when the output is an enum.
    // We will try first a int convertion
    if constexpr(std::is_enum_v<T>) {
        if(IsNumber()) {
            return static_cast<T>(Convert<int>().value());
        }
        if(IsString()) {
            if(auto out = StringToNumber<int64_t>()) {
                return static_cast<T>(out.value());
            }
        }
        return nonstd::make_unexpected("Any::cast failed to Cast to enum Type");
    }

    if(IsString()) {
        if constexpr(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>) {
            if(auto out = StringToNumber<T>()) {
                return out.value();
            } else {
                return out;
            }
        }
    }

    if(auto res = Convert<T>()) {
        return res.value();
    } else {
        return res;
    }
}

}// namespace behaviortree

#endif// BEHAVIORTREE_SAFE_ANY_HPP
