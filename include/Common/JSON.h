//
// Created by Renatus Madrigal on 01/18/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_JSON_H_
#define TINY_COBALT_INCLUDE_COMMON_JSON_H_

#include <concepts>
#include <nlohmann/json.hpp>
#include <proxy.h>
#include <utility>

namespace TinyCobalt::Common {
    using JSON = nlohmann::json;

    PRO_DEF_MEM_DISPATCH(MemToJSON, toJSON);

    struct ToJSONProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemToJSON, JSON() const> // NOLINT
          ::build {};

    // Interface for json deserialization
    template<typename T>
    T fromJSON(const Common::JSON &obj);

    template<typename T>
    concept ToJSONConcept = pro::proxiable<T *, ToJSONProxy>;

    template<typename T>
    concept JSONSerializable = requires(T obj) {
        { obj.toJSON() } -> std::same_as<JSON>;
        { fromJSON<T>(std::declval<JSON>()) } -> std::same_as<T>;
    };

    using nlohmann::operator""_json;
    using nlohmann::operator""_json_pointer;

} // namespace TinyCobalt::Common

#endif // TINY_COBALT_INCLUDE_COMMON_JSON_H_