//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_UTILITY_H_
#define TINY_COBALT_INCLUDE_COMMON_UTILITY_H_

#include <string>
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

    namespace detail {
        template<typename... Args>
        struct MergedVariantHelper;
        template<typename... VTs>
        struct MergedVariantHelper<std::variant<VTs...>> {
            using type = std::variant<VTs...>;
        };
        template<typename T>
        struct MergedVariantHelper<T> {
            using type = std::variant<T>;
        };
        template<typename... VT1s, typename... VT2s>
        struct MergedVariantHelper<std::variant<VT1s...>, std::variant<VT2s...>> {
            using type = std::variant<VT1s..., VT2s...>;
        };
        template<typename T, typename... Ts>
        struct MergedVariantHelper<T, Ts...> {
            using type = typename MergedVariantHelper<typename MergedVariantHelper<T>::type,
                                                      typename MergedVariantHelper<Ts...>::type>::type;
        };
        template<typename T>
        struct UniqueVariantHelper {
            using type = std::variant<T>;
        };
        template<typename T>
        struct UniqueVariantHelper<std::variant<T>> {
            using type = std::variant<T>;
        };
        template<typename T, typename... Ts>
        struct UniqueVariantHelper<std::variant<T, Ts...>> {
            using type = std::conditional_t<
                    (std::is_same_v<T, Ts> || ...), typename UniqueVariantHelper<std::variant<Ts...>>::type,
                    typename MergedVariantHelper<T, typename UniqueVariantHelper<std::variant<Ts...>>::type>::type>;
        };


    } // namespace detail

    template<typename... Ts>
    using UnionedVariant =
            typename detail::UniqueVariantHelper<typename detail::MergedVariantHelper<Ts...>::type>::type;

    // tests for MergedVariant
    static_assert(std::is_same_v<UnionedVariant<bool, std::variant<int>, std::variant<double>>,
                                 std::variant<bool, int, double>>);
    static_assert(std::is_same_v<UnionedVariant<std::variant<int, bool>, std::variant<double, std::string>>,
                                 std::variant<int, bool, double, std::string>>);
    static_assert(std::is_same_v<UnionedVariant<std::variant<int, double>>, std::variant<int, double>>);
    static_assert(std::is_same_v<UnionedVariant<std::variant<int, double>, std::variant<bool, double>>,
                                 std::variant<int, bool, double>>);

    template<typename T, typename U>
    inline constexpr bool IsVariantMember = false;
    template<typename T, typename... VTs>
    inline constexpr bool IsVariantMember<T, std::variant<VTs...>> = (std::is_same_v<T, VTs> || ...);
    
} // namespace TinyCobalt::Utility

#endif // TINY_COBALT_INCLUDE_COMMON_UTILITY_H_