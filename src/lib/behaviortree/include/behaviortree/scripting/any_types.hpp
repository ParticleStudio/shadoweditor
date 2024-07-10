#ifndef BEHAVIORTREE_ANY_TYPES_HPP
#define BEHAVIORTREE_ANY_TYPES_HPP

#include "behaviortree/util/safe_any.hpp"
#include "lexy/action/parse.hpp"
#include "lexy/callback.hpp"
#include "lexy/dsl.hpp"
#include "lexy_ext/report_error.hpp"

namespace behaviortree::Grammar {
namespace dsl = lexy::dsl;

struct _xid_start_character: lexyd::char_class_base<_xid_start_character> {
    static LEXY_CONSTEVAL auto char_class_name() {
        return "code-point.BT-start-character";
    }

    static LEXY_CONSTEVAL auto char_class_ascii() {
        lexy::_detail::ascii_set result;
        result.insert('a', 'z');
        result.insert('A', 'Z');
        result.insert('_');
        result.insert('@');
        return result;
    }

    static LEXY_UNICODE_CONSTEXPR bool char_class_match_cp(char32_t cp) {
        // underscore handled as part of ASCII.
        return lexy::_detail::code_point_has_properties<
                LEXY_UNICODE_PROPERTY(xid_start)>(cp);
    }

    template<typename Encoding>
    static constexpr auto char_class_match_swar(lexy::_detail::swar_int c) {
        return lexyd::ascii::_alphau::template char_class_match_swar<Encoding>(c
        );
    }
};
inline constexpr auto xid_start_character = _xid_start_character{};

// A Unicode-aware identifier.
struct Name {
    static constexpr auto rule =
            dsl::identifier(xid_start_character, dsl::unicode::xid_continue);

    static constexpr auto value = lexy::as_string<std::string>;
};

//----------
struct Integer: lexy::token_production {
    struct integer {
        static constexpr auto rule = dsl::sign + dsl::integer<int64_t>;
        static constexpr auto value = lexy::as_integer<int64_t>;
    };

    struct invalid_suffix {
        static constexpr auto name = "invalid suffix on integer literal";
    };

    static constexpr auto rule = [] {
        auto hex_integer = (LEXY_LIT("0x") | LEXY_LIT("0X")) >>
                           dsl::integer<int64_t, dsl::hex>;
        auto regular_integer =
                dsl::peek(dsl::lit_c<'-'> / dsl::lit_c<'+'> / dsl::digit<>) >>
                dsl::p<integer>;
        auto suffix_error = dsl::peek_not(
                                    dsl::period / dsl::ascii::alpha /
                                    dsl::ascii::alpha_underscore
        )
                                    .error<invalid_suffix>;
        return (hex_integer | regular_integer) >> suffix_error;
    }();

    static constexpr auto value = lexy::construct<int64_t>;
};

//----------
struct Real: lexy::token_production {
    struct invalid_suffix {
        static constexpr auto name = "invalid suffix on double literal";
    };

    static constexpr auto rule = [] {
        auto integer_part = dsl::sign + dsl::digits<>;
        auto fraction = dsl::period >> dsl::digits<>;
        auto exponent = (dsl::lit_c<'e'> / dsl::lit_c<'E'>) >>
                        (dsl::sign + dsl::digits<>);

        auto suffix_error = dsl::peek_not(
                                    dsl::period / dsl::ascii::alpha /
                                    dsl::ascii::alpha_underscore
        )
                                    .error<invalid_suffix>;

        auto real_part =
                (fraction >> dsl::if_(exponent) | exponent) >> suffix_error;
        auto real_number = dsl::token(integer_part + real_part);
        return dsl::capture(real_number);
    }();

    static constexpr auto value =
            lexy::as_string<std::string> |
            lexy::callback<behaviortree::Any>([](std::string&& str) {
                return behaviortree::Any(std::stod(str));
            });
};

//----------
//struct Variable : lexy::token_production
//{
//  static constexpr auto rule = dsl::identifier(dsl::unicode::xid_start_underscore, dsl::unicode::xid_continue);
//  static constexpr auto value = lexy::as_string<std::string>;
//};

//----------
struct StringLiteral: lexy::token_production {
    static constexpr auto rule = dsl::single_quoted(dsl::ascii::character) |
                                 dsl::quoted(dsl::ascii::character);
    static constexpr auto value = lexy::as_string<std::string>;
};

//----------
struct BooleanLiteral: lexy::token_production {
    struct True {
        static constexpr auto rule = LEXY_LIT("true");
        static constexpr auto value = lexy::constant(1);
    };
    struct False {
        static constexpr auto rule = LEXY_LIT("false");
        static constexpr auto value = lexy::constant(0);
    };

    static constexpr auto rule = dsl::p<True> | dsl::p<False>;
    static constexpr auto value = lexy::construct<behaviortree::Any>;
};

//----------
struct AnyValue: lexy::token_production {
    static constexpr auto rule = dsl::p<BooleanLiteral> |
                                 dsl::p<StringLiteral> | dsl::p<Real> |
                                 dsl::p<Integer>;
    static constexpr auto value = lexy::construct<behaviortree::Any>;
};

}// namespace behaviortree::Grammar

#endif// BEHAVIORTREE_ANY_TYPES_HPP
