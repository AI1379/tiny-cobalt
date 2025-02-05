//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_UTILITY_H_
#define TINY_COBALT_INCLUDE_COMMON_UTILITY_H_

#include <concepts>
#include <optional>
#include <proxy.h>
#include <string>
#include <tuple>
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

    // TODO: add a restrict to the type of the proxy
    template<typename T, typename P>
    bool containType(pro::proxy<P> proxy) {
        return typeid(T).hash_code() == proxy_typeid(*proxy).hash_code();
    }

    template<typename T, typename P>
    bool pointerType(pro::proxy<P> proxy) {
        return typeid(T).hash_code() == proxy_typeid(proxy).hash_code();
    }

    // Implemented reflection for proxy, might be simplified with the upcoming C++26
    // See https://github.com/microsoft/proxy/issues/237#issuecomment-2623871501
    // TODO: Implementation details of ms-proxy are used. Need to remove them.
    namespace details {

        // Reflectors for weak_dispatch with cvref and noexcept qualifiers
        template<class D, class O>
        struct single_weak_impl_reflector;
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...)> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) &> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) & noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) &&> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, T &&, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) && noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, T &&, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, const T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, const T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const &> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, const T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const & noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, const T &, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const &&> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_invocable_r_v<R, D, const T &&, Args...>) {}
            bool value;
        };
        template<class D, class R, class... Args>
        struct single_weak_impl_reflector<D, R(Args...) const && noexcept> {
            template<class T>
            constexpr explicit single_weak_impl_reflector(std::in_place_type_t<T>) :
                value(std::is_nothrow_invocable_r_v<R, D, const T &&, Args...>) {}
            bool value;
        };

        // Implementation of weak_impl_reflector
        template<class... Rs>
        struct weak_impl_reflector_impl : Rs... {
            template<class T>
            constexpr explicit weak_impl_reflector_impl(std::in_place_type_t<T>) : Rs(std::in_place_type<T>)... {}

            template<class F, bool IsDirect, class R>
            struct accessor {
                template<class D, class O>
                bool implemented() const noexcept {
                    using Impl = single_weak_impl_reflector<D, O>;
                    static_assert(std::is_base_of_v<Impl, weak_impl_reflector_impl>,
                                  "this function only provides query for weak_dispatch");
                    return static_cast<const Impl &>(pro::proxy_reflect<IsDirect, R>(pro::access_proxy<F>(*this)))
                            .value;
                }
            };
        };

        // TODO: figure out how this works
        template<class O, class I>
        struct merge_weak_impl_reflector_reduction;
        template<class... Rs1, class... Rs2>
        struct merge_weak_impl_reflector_reduction<weak_impl_reflector_impl<Rs1...>, weak_impl_reflector_impl<Rs2...>>
            : std::type_identity<weak_impl_reflector_impl<Rs1..., Rs2...>> {};

        template<class D, class... Os>
        using compact_weak_impl_reflector = weak_impl_reflector_impl<single_weak_impl_reflector<D, Os>...>;

        template<class O, class C, class D>
        struct weak_impl_reduction_impl : std::type_identity<O> {};
        template<class O, class C, class D>
        struct weak_impl_reduction_impl<O, C, pro::weak_dispatch<D>>
            : merge_weak_impl_reflector_reduction<
                      O, pro::details::instantiated_t<compact_weak_impl_reflector, typename C::overload_types, D>> {};
        template<class O, class I>
        struct weak_impl_reduction : weak_impl_reduction_impl<O, I, typename I::dispatch_type> {};
        template<class O, class I>
        using weak_impl_reduction_t = typename weak_impl_reduction<O, I>::type;

        template<class... Cs>
        using weak_impl_recursive_reduction_t =
                pro::details::recursive_reduction_t<weak_impl_reduction_t, weak_impl_reflector_impl<>, Cs...>;

        template<class F>
        struct weak_impl_reflector_impl_traits
            : std::type_identity<
                      pro::details::instantiated_t<weak_impl_recursive_reduction_t, typename F::convention_types>> {};
        template<class F>
        using weak_impl_reflector_impl_t = typename weak_impl_reflector_impl_traits<F>::type;

    } // namespace details

    template<class F>
    struct WeakImplementedRefl : details::weak_impl_reflector_impl_t<F> {
        template<class T>
        constexpr explicit WeakImplementedRefl(std::in_place_type_t<T>) :
            details::weak_impl_reflector_impl_t<F>(std::in_place_type<T>) {}
    };

    template<typename T>
    concept RttiAwareProxy = requires(T proxy) {
        { proxy_typeid(*proxy) } -> std::same_as<const std::type_info &>;
    };


    // FIXME: Failed to handle unique_ptr
    template<typename F, typename... Fs>
        requires(std::is_same_v<typename function_traits<F>::result_type, // NOLINT
                                typename function_traits<Fs>::result_type> &&
                 ...)
    struct Matcher : F, Fs... {
        using Result = typename function_traits<F>::result_type;

        static_assert(std::is_default_constructible_v<Result> || std::is_same_v<Result, void>,
                      "Result type should be default constructible or void");

        template<typename T>
            requires RttiAwareProxy<T>
        Result operator()(T &&op) const {
            if constexpr (!std::is_same_v<Result, void>) {
                Result result;
                try_invoke(&result, static_cast<F>(*this), op);
                (try_invoke(&result, static_cast<Fs>(*this), op), ...);
                return result;
            } else {
                try_invoke_void(static_cast<F>(*this), op);
                (try_invoke_void(static_cast<Fs>(*this), op), ...);
            }
        }

        template<typename O, typename T>
            requires(RttiAwareProxy<T> && !std::is_same_v<Result, void>)
        void try_invoke(Result *res, O &&f, T &&op) const {
            auto type_hash = proxy_typeid(op).hash_code();
            using Arg = std::remove_cvref_t<std::tuple_element_t<0, typename function_traits<O>::args_type>>;
            if (typeid(Arg).hash_code() == type_hash) {
                *res = f(proxy_cast<Arg>(op));
            }
        }

        template<typename O, typename T>
            requires RttiAwareProxy<T>
        void try_invoke_void(O &&f, T &&op) const {
            auto type_hash = proxy_typeid(op).hash_code();
            using Arg = std::remove_cvref_t<std::tuple_element_t<0, typename function_traits<O>::args_type>>;
            if (typeid(Arg).hash_code() == type_hash) {
                f(proxy_cast<Arg>(op));
            }
        }
    };

    template<typename... Fs>
    Matcher(Fs...) -> Matcher<Fs...>;

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_UTILITY_H_