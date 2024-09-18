module behaviortree.switch_node;

#if __has_include(<charconv>)
import <charconv>;
#endif

namespace behaviortree::details {

bool CheckStringEquality(const std::string &rStr, const std::string &rResult, const ScriptingEnumsRegistry *pEnums) {
    // compare strings first
    if(rStr == rResult) {
        return true;
    }
    // compare as integers next
    auto ToInt = [pEnums](const std::string &refStr, auto &refResult) -> bool {
        if(pEnums) {
            auto it = pEnums->find(refStr);
            if(it != pEnums->end()) {
                refResult = it->second;
                return true;
            }
        }
#if __cpp_lib_to_chars >= 201611L
        auto [ptr, ec] = std::from_chars(
                refStr.data(), refStr.data() + refStr.size(), refResult
        );
        return (ec == std::errc());
#else
        try {
            result = std::stoi(refStr);
            return true;
        } catch(...) {
            return false;
        }
#endif
    };
    int v1Int = 0;
    int v2Int = 0;
    if(ToInt(rStr, v1Int) && ToInt(rResult, v2Int) && v1Int == v2Int) {
        return true;
    }
    // compare as real numbers next
    auto ToReal = [](const std::string &refStr, auto &refResult) -> bool {
#if __cpp_lib_to_chars >= 201611L
        auto [ptr, ec] = std::from_chars(
                refStr.data(), refStr.data() + refStr.size(), refResult
        );
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
    if(ToReal(rStr, v1Real) && ToReal(rResult, v2Real) && std::abs(v1Real - v2Real) <= eps) {
        return true;
    }
    return false;
}

}// namespace behaviortree::details
