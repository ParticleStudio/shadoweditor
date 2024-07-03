#pragma once

#include "behaviortree/basic_types.h"
#include "behaviortree/util/safe_any.hpp"
#include "behaviortree/basic_types.h"

// Use the version nlohmann::json embedded in BT.CPP
#include "behaviortree/contrib/json.hpp"

namespace BT
{

/**
*  To add new type to the JSON library, you should follow these isntructions:
*    https://json.nlohmann.me/features/arbitrary_types/
*
*  Considering for instance the type:
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
*  Later, you MUST register the type using:
*
*  BT::RegisterJsonDefinition<Point2D>();
*/

//-----------------------------------------------------------------------------------

/**
*  Use RegisterJsonDefinition<Foo>();
*/

class JsonExporter
{
public:
  static JsonExporter& get();

  /**
   * @brief toJson adds the content of "any" to the JSON "destination".
   *
   * It will return false if the conversion toJson is not possible
   * If it is a custom type, you might register it first with addConverter().
   */
  bool toJson(const BT::Any& any, nlohmann::json& destination) const;

  /// This information is needed to create a BT::Blackboard::entry
  using Entry = std::pair<BT::Any, BT::TypeInfo>;

  using ExpectedEntry = nonstd::expected_lite::expected<Entry, std::string>;

  /**
   * @brief fromJson will return an Entry (value wrappedn in Any + TypeInfo)
   * from a json source.
   * If it is a custom type, you might register it first with addConverter().
   * @param source
   * @return
   */
  ExpectedEntry fromJson(const nlohmann::json& source) const;

  /// Same as the other, but providing the specific type,
  /// To be preferred if the JSON doesn't contain the field [__type]
  ExpectedEntry fromJson(const nlohmann::json& source, std::type_index type) const;

  template <typename T>
  Expected<T> fromJson(const nlohmann::json& source) const;

  /// Register new JSON converters with addConverter<Foo>().
  /// You should have used first the macro BT_JSON_CONVERTER
  template <typename T>
  void addConverter();

  /**
   * @brief addConverter register a to_json function that converts a json to a type T.
   *
   * @param to_json the function with signature void(const T&, nlohmann::json&)
   * @param add_type if true, add a field called [__type] with the name ofthe type.
   * */
  template <typename T>
  void addConverter(std::function<void(const T&, nlohmann::json&)> to_json,
                    bool add_type = true);

  /// Register custom from_json converter directly.
  template <typename T>
  void addConverter(std::function<void(const nlohmann::json&, T&)> from_json);

private:
  using ToJonConverter = std::function<void(const BT::Any&, nlohmann::json&)>;
  using FromJonConverter = std::function<Entry(const nlohmann::json&)>;

  std::unordered_map<std::type_index, ToJonConverter> to_json_converters_;
  std::unordered_map<std::type_index, FromJonConverter> from_json_converters_;
  std::unordered_map<std::string, BT::TypeInfo> type_names_;
};

template <typename T>
inline Expected<T> JsonExporter::fromJson(const nlohmann::json& source) const
{
  auto res = fromJson(source);
  if(!res)
  {
    return nonstd::expected_lite::make_unexpected(res.error());
  }
  auto casted = res->first.tryCast<T>();
  if(!casted)
  {
    return nonstd::expected_lite::make_unexpected(casted.error());
  }
  return *casted;
}

//-------------------------------------------------------------------

template <typename T>
inline void JsonExporter::addConverter()
{
  ToJonConverter to_converter = [](const BT::Any& entry, nlohmann::json& dst) {
    dst = *const_cast<BT::Any&>(entry).castPtr<T>();
  };
  to_json_converters_.insert({ typeid(T), to_converter });

  FromJonConverter from_converter = [](const nlohmann::json& src) -> Entry {
    T value = src.get<T>();
    return { BT::Any(value), BT::TypeInfo::Create<T>() };
  };

  // we need to get the name of the type
  nlohmann::json const js = T{};
  // we insert both the name obtained from JSON and demangle
  if(js.contains("__type"))
  {
    type_names_.insert({ std::string(js["__type"]), BT::TypeInfo::Create<T>() });
  }
  type_names_.insert({ BT::demangle(typeid(T)), BT::TypeInfo::Create<T>() });

  from_json_converters_.insert({ typeid(T), from_converter });
}

template <typename T>
inline void JsonExporter::addConverter(
    std::function<void(const T&, nlohmann::json&)> func, bool add_type)
{
  auto converter = [func, add_type](const BT::Any& entry, nlohmann::json& json) {
    func(entry.cast<T>(), json);
    if(add_type)
    {
      json["__type"] = BT::demangle(typeid(T));
    }
  };
  to_json_converters_.insert({ typeid(T), std::move(converter) });
}

template <typename T>
inline void
JsonExporter::addConverter(std::function<void(const nlohmann::json&, T&)> func)
{
  auto converter = [func](const nlohmann::json& json) -> Entry {
    T tmp;
    func(json, tmp);
    return { BT::Any(tmp), BT::TypeInfo::Create<T>() };
  };
  type_names_.insert({ BT::demangle(typeid(T)), BT::TypeInfo::Create<T>() });
  from_json_converters_.insert({ typeid(T), std::move(converter) });
}

template <typename T>
inline void RegisterJsonDefinition()
{
  JsonExporter::get().addConverter<T>();
}

}  // namespace BT

//------------------------------------------------
//------------------------------------------------
//------------------------------------------------

// Macro to implement to_json() and from_json()

#define BT_JSON_CONVERTER(Type, value)                                                   \
  template <class AddField>                                                              \
  void _JsonTypeDefinition(Type&, AddField&);                                            \
                                                                                         \
  inline void to_json(nlohmann::json& js, const Type& p)                                 \
  {                                                                                      \
    auto op = [&js](const char* name, auto* val) { js[name] = *val; };                   \
    _JsonTypeDefinition(const_cast<Type&>(p), op);                                       \
    js["__type"] = #Type;                                                                \
  }                                                                                      \
                                                                                         \
  inline void from_json(const nlohmann::json& js, Type& p)                               \
  {                                                                                      \
    auto op = [&js](const char* name, auto* v) { js.at(name).get_to(*v); };              \
    _JsonTypeDefinition(p, op);                                                          \
  }                                                                                      \
                                                                                         \
  template <class AddField>                                                              \
  inline void _JsonTypeDefinition(Type& value, AddField& add_field)

//end of file
