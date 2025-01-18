//
// Created by Renatus Madrigal on 01/18/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_JSON_H_
#define TINY_COBALT_INCLUDE_COMMON_JSON_H_

#include <nlohmann/json.hpp>
#include <proxy.h>

namespace TinyCobalt::Common {
    using JSON = nlohmann::json;

    PRO_DEF_MEM_DISPATCH(MemToJSON, toJSON);

    struct ToJSONProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemToJSON, JSON() const> // NOLINT
          ::build {};

    template<typename T>
    concept ToJSONConcept = pro::proxiable<T *, ToJSONProxy>;

} // namespace TinyCobalt::Common

#endif // TINY_COBALT_INCLUDE_COMMON_JSON_H_