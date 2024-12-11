//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_UTILITY_H_
#define TINY_COBALT_INCLUDE_COMMON_UTILITY_H_

#include <type_traits>
#include <variant>

#include <proxy.h>

namespace TinyCobalt::Utility {
    template<typename T>
    struct is_variant : std::false_type {};
    template<typename... Ts>
    struct is_variant<std::variant<Ts...>> : std::true_type {};
    template<typename... Ts>
    struct is_variant<std::variant<Ts...> const> : std::true_type {};
    template<typename T>
    inline constexpr bool is_variant_v = is_variant<T>::value;
    template<typename T>
    concept Variant = is_variant_v<T>;

    template<typename T, typename... Ts>
    struct MergedVariant {
        using type = std::variant<T, typename MergedVariant<Ts...>::type>;
    };
    template<typename... VTs, typename... Ts>
    struct MergedVariant<std::variant<VTs...>, Ts...> {
        using type = std::variant<VTs..., typename MergedVariant<Ts...>::type>;
    };
    template<typename... VT1s, typename... VT2s>
    struct MergedVariant<std::variant<VT1s...>, std::variant<VT2s...>> {
        using type = std::variant<VT1s..., VT2s...>;
    };

    PRO_DEF_FREE_DISPATCH(FreeToDump, dump);
    struct DumpableProxy : pro::facade_builder::add_convention<FreeToDump, void(std::ostream &)>::build {};
    template<typename T>
    concept Dumpable = pro::proxiable<T, DumpableProxy>;

} // namespace TinyCobalt::Utility

#endif // TINY_COBALT_INCLUDE_COMMON_UTILITY_H_