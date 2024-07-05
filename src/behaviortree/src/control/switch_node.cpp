#include "behaviortree/control/switch_node.h"

#if __has_include(<charconv>)
#include <charconv>
#endif

namespace behaviortree::details {

bool CheckStringEquality(const std::string& refV1, const std::string& refV2,
                         const ScriptingEnumsRegistry* enums) {
    // compare strings first
    if(refV1 == refV2) {
        return true;
    }
    // compare as integers next
    auto ToInt = [enums](const std::string& refStr, auto& refResult) -> bool {
        if(enums) {
            auto it = enums->find(refStr);
            if(it != enums->end()) {
                refResult = it->second;
                return true;
            }
        }
#if __cpp_lib_to_chars >= 201611L
        auto [ptr, ec] = std::from_chars(refStr.data(), refStr.data() + refStr.size(), refResult);
        return (ec == std::errc());
#else
        try {
            result = std::stoi(str);
            return true;
        } catch(...) {
            return false;
        }
#endif
    };
    int v1Int = 0;
    int v2Int = 0;
    if(ToInt(refV1, v1Int) && ToInt(refV2, v2Int) && v1Int == v2Int) {
        return true;
    }
    // compare as real numbers next
    auto ToReal = [](const std::string& refStr, auto& refResult) -> bool {
#if __cpp_lib_to_chars >= 201611L
        auto [ptr, ec] = std::from_chars(refStr.data(), refStr.data() + refStr.size(), refResult);
        return (ec == std::errc());
#else
        try {
            result = std::stod(str);
            return true;
        } catch(...) {
            return false;
        }
#endif
    };
    double v1Real = 0;
    double v2Real = 0;
    constexpr auto eps = double(std::numeric_limits<float>::epsilon());
    if(ToReal(refV1, v1Real) && ToReal(refV2, v2Real) && std::abs(v1Real - v2Real) <= eps) {
        return true;
    }
    return false;
}

}// namespace behaviortree::details
