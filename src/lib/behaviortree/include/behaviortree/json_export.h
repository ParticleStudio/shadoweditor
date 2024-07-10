#ifndef BEHAVIORTREE_JSON_EXPORT_H
#define BEHAVIORTREE_JSON_EXPORT_H

#include "behaviortree/basic_types.h"
#include "behaviortree/util/safe_any.hpp"

// Use the version nlohmann::json embedded in BT.CPP
#include "behaviortree/contrib/json.hpp"

namespace behaviortree {

/**
*  To Add new Type to the JSON library, you should follow these isntructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  Considering for instance the Type:
*
*   struct Point2D {
*     double x;
*     double y;
*   };
*
*  This would require the implementation of:
*
*   void to_json(nlohmann::json& j, const Point2D& point);
*   void from_json(const nlohmann::json& j, Point2D& point);
*
*  To avoid repeating yourself, we provide the macro BT_JSON_CONVERTION
*  that implements both those function, at once. Usage:
*
*  BT_JSON_CONVERTER(Point2D, point)
*  {
*     add_field("x", &point.x);
*     add_field("y", &point.y);
*  }
*
*  Later, you MUST register the Type using:
*
*  BT::RegisterJsonDefinition<Point2D>();
*/

//-----------------------------------------------------------------------------------

/**
*  Use RegisterJsonDefinition<Foo>();
*/

class JsonExporter {
 public:
    static JsonExporter& Get();

    /**
   * @brief toJson adds the content of "any" to the JSON "destination".
   *
   * It will return false if the conversion toJson is not possible
   * If it is a custom Type, you might register it first with addConverter().
   */
    bool ToJson(const behaviortree::Any& refAny, nlohmann::json& refDst) const;

    /// This information is needed to create a BT::Blackboard::entry
    using Entry = std::pair<behaviortree::Any, behaviortree::TypeInfo>;

    using ExpectedEntry = nonstd::expected<Entry, std::string>;

    /**
   * @brief fromJson will return an Entry (value wrappedn in Any + TypeInfo)
   * from a json source.
   * If it is a custom Type, you might register it first with addConverter().
   * @param source
   * @return
   */
    ExpectedEntry FromJson(const nlohmann::json& refSource) const;

    /// Same as the other, but providing the specific Type,
    /// To be preferred if the JSON doesn't contain the field [__type]
    ExpectedEntry FromJson(
            const nlohmann::json& refSource, std::type_index type
    ) const;

    template<typename T>
    Expected<T> FromJson(const nlohmann::json& refSource) const;

    /// Register new JSON converters with addConverter<Foo>().
    /// You should have used first the macro BT_JSON_CONVERTER
    template<typename T>
    void AddConverter();

    /**
   * @brief addConverter register a to_json function that converts a json to a Type T.
   *
   * @param to_json the function with signature void(const T&, nlohmann::json&)
   * @param add_type if true, Add a field called [__type] with the name ofthe Type.
   * */
    template<typename T>
    void AddConverter(
            std::function<void(const T&, nlohmann::json&)> toJson,
            bool addType = true
    );

    /// Register custom from_json converter directly.
    template<typename T>
    void addConverter(std::function<void(const nlohmann::json&, T&)> fromJson);

 private:
    using ToJonConverter =
            std::function<void(const behaviortree::Any&, nlohmann::json&)>;
    using FromJonConverter = std::function<Entry(const nlohmann::json&)>;

    std::unordered_map<std::type_index, ToJonConverter> m_ToJsonConverters;
    std::unordered_map<std::type_index, FromJonConverter> m_FromJsonConverters;
    std::unordered_map<std::string, behaviortree::TypeInfo> m_TypeNames;
};

template<typename T>
inline Expected<T> JsonExporter::FromJson(const nlohmann::json& refSource
) const {
    auto res = FromJson(refSource);
    if(!res) {
        return nonstd::expected_lite::make_unexpected(res.error());
    }
    auto casted = res->first.TryCast<T>();
    if(!casted) {
        return nonstd::expected_lite::make_unexpected(casted.error());
    }
    return *casted;
}

//-------------------------------------------------------------------

template<typename T>
inline void JsonExporter::AddConverter() {
    ToJonConverter toConverter = [](const behaviortree::Any& refEntry,
                                    nlohmann::json& refDst) {
        refDst = *const_cast<behaviortree::Any&>(refEntry).CastPtr<T>();
    };
    m_ToJsonConverters.insert({typeid(T), toConverter});

    FromJonConverter fromConverter = [](const nlohmann::json& refSrc) -> Entry {
        T value = refSrc.get<T>();
        return {behaviortree::Any(value), behaviortree::TypeInfo::Create<T>()};
    };

    // we need to get the name of the type
    nlohmann::json const js = T{};
    // we insert both the name obtained from JSON and demangle
    if(js.contains("__type")) {
        m_TypeNames.insert(
                {std::string(js["__type"]), behaviortree::TypeInfo::Create<T>()}
        );
    }
    m_TypeNames.insert(
            {behaviortree::Demangle(typeid(T)),
             behaviortree::TypeInfo::Create<T>()}
    );

    m_FromJsonConverters.insert({typeid(T), fromConverter});
}

template<typename T>
inline void JsonExporter::AddConverter(
        std::function<void(const T&, nlohmann::json&)> func, bool addType
) {
    auto converter = [func, addType](
                             const behaviortree::Any& refEntry,
                             nlohmann::json& refJson
                     ) {
        func(refEntry.Cast<T>(), refJson);
        if(addType) {
            refJson["__type"] = behaviortree::Demangle(typeid(T));
        }
    };
    m_ToJsonConverters.insert({typeid(T), std::move(converter)});
}

template<typename T>
inline void JsonExporter::addConverter(
        std::function<void(const nlohmann::json&, T&)> func
) {
    auto converter = [func](const nlohmann::json& refJson) -> Entry {
        T tmp;
        func(refJson, tmp);
        return {behaviortree::Any(tmp), behaviortree::TypeInfo::Create<T>()};
    };
    m_TypeNames.insert(
            {behaviortree::Demangle(typeid(T)),
             behaviortree::TypeInfo::Create<T>()}
    );
    m_FromJsonConverters.insert({typeid(T), std::move(converter)});
}

template<typename T>
inline void RegisterJsonDefinition() {
    JsonExporter::Get().AddConverter<T>();
}

}// namespace behaviortree

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

// Macro to implement to_json() and from_json()

#define BT_JSON_CONVERTER(Type, value)                              \
    template<class AddField>                                        \
    void JsonTypeDefinition(Type&, AddField&);                      \
                                                                    \
    inline void ToJson(nlohmann::json& refJs, const Type& refP) {   \
        auto op = [&refJs](const char* ptrName, auto* ptrVal) {     \
            js[ptrName] = *ptrVal;                                  \
        };                                                          \
        JsonTypeDefinition(const_cast<Type&>(refP), op);            \
        js["__type"] = #Type;                                       \
    }                                                               \
                                                                    \
    inline void FromJson(const nlohmann::json& refJs, Type& refP) { \
        auto op = [&refJs](const char* ptrName, auto* ptrV) {       \
            js.at(ptrName).get_to(*ptrV);                           \
        };                                                          \
        JsonTypeDefinition(refP, op);                               \
    }                                                               \
                                                                    \
    template<class AddField>                                        \
    inline void JsonTypeDefinition(Type& refValue, AddField& refAddField)

#endif// BEHAVIORTREE_JSON_EXPORT_H
