//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_UTILITY_H_
#define TINY_COBALT_INCLUDE_COMMON_UTILITY_H_

#include <concepts>
#include <proxy.h>
#include <string>
#include <type_traits>
#include <variant>

namespace TinyCobalt {
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

    template<typename Alloc, typename T>
    using AllocRebind = std::allocator_traits<Alloc>::template rebind_alloc<T>;

    template<typename InputIter>
    using IteratorKeyType = std::remove_const_t<typename std::iterator_traits<InputIter>::value_type::first_type>;

    template<typename InputIter>
    using IteratorValueType = std::remove_const_t<typename std::iterator_traits<InputIter>::value_type::second_type>;

    template<bool IsConst, typename T>
    using ConditionalConst = std::conditional_t<IsConst, const T, T>;

    namespace detail {
        template<typename T>
        concept BooleanTestableImpl = std::convertible_to<T, bool>;

        template<typename T>
        concept BooleanTestable = BooleanTestableImpl<T> && requires(T &&t) {
            { !static_cast<T &&>(t) } -> BooleanTestableImpl;
        };
    } // namespace detail

    inline constexpr struct {
        template<typename T, typename U>
        static constexpr bool noexceptTest(const T *t = nullptr, const U *u = nullptr) {
            if constexpr (std::three_way_comparable_with<T, U>) {
                return noexcept(*t <=> *u);
            } else {
                return noexcept(*t < *u) && noexcept(*u < *t);
            }
        }

        template<typename T, typename U>
        [[nodiscard]] constexpr auto operator()(const T &t, const U &u) const noexcept(noexceptTest<T, U>())
            requires requires {
                { t < u } -> detail::BooleanTestable;
                { u < t } -> detail::BooleanTestable;
            }
        {
            if constexpr (std::three_way_comparable_with<T, U>) {
                return t <=> u;
            } else {
                if (t < u) {
                    return std::weak_ordering::less;
                } else if (u < t) {
                    return std::weak_ordering::greater;
                } else {
                    return std::weak_ordering::equivalent;
                }
            }
        }
    } kSynth3wayComparator = {};

    template<typename T, typename U = T>
    using Synth3way = decltype(kSynth3wayComparator(std::declval<T &>(), std::declval<U &>()));


    // Primary template: fall back to operator() if T is callable
    template<typename T>
    struct function_traits : function_traits<decltype(&T::operator())> {};

    // Specialization for a plain function pointer
    template<typename R, typename... Args>
    struct function_traits<R (*)(Args...)> {
        using result_type = R;
        using args_type = std::tuple<Args...>;
    };

    template<typename R, typename... Args>
    struct function_traits<R(Args...)> {
        using result_type = R;
        using args_type = std::tuple<Args...>;
    };

    // Specialization for a non-static member function pointer (const-qualified)
    template<typename R, typename C, typename... Args>
    struct function_traits<R (C::*)(Args...) const> {
        using result_type = R;
        using args_type = std::tuple<Args...>;
    };

    struct RttiAware : pro::facade_builder // NOLINT
                       ::support_rtti // NOLINT
                       ::build {};

    // TODO: add a restrict to the type of the proxy
    template<typename T, typename P>
    bool containType(pro::proxy<P> proxy) {
        return typeid(T).hash_code() == proxy_typeid(*proxy).hash_code();
    }

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_UTILITY_H_