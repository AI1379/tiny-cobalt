//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_UTILITY_H_
#define TINY_COBALT_INCLUDE_COMMON_UTILITY_H_

#include <concepts>
#include <cstddef>
#include <proxy.h>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>
#include "Common/Concept.h"

namespace TinyCobalt {
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
    using UnionVariant = typename detail::UniqueVariantHelper<typename detail::MergedVariantHelper<Ts...>::type>::type;

    template<typename T, typename U>
    inline constexpr bool IsVariantMember = false;
    template<typename T, typename... VTs>
    inline constexpr bool IsVariantMember<T, std::variant<VTs...>> = (std::is_same_v<T, VTs> || ...);

    // TODO: Implement a VariantWithConcept

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

#ifdef __cpp_pack_indexing
    template<size_t kIdx, typename... Ts>
    using PackIndex = Ts...[0];
#else
    template<size_t kIdx, typename... Ts>
    using PackIndex = std::tuple_element_t<kIdx, std::tuple<Ts...>>;
#endif

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

    template<typename M>
    struct MatcherTraits {
        using Result = typename M::Result;
        using CandidateArgs = typename M::CandidateArgs;
        // TODO: It may be possible to implement this using C++26
        // using CandidateArgs = std::tuple < candidate arguments >;
    };

    // TODO: Implement Matcher concept.
    template<typename M>
    concept MatcherConcept = requires(M m) { typename MatcherTraits<M>::Result; };

    namespace detail {
        template<typename Arg, typename Result, typename O, typename T>
            requires RttiAwareProxy<T>
        void try_invoke(Result *res, O &&f, T &&op) {
            auto type_hash = proxy_typeid(op).hash_code();
            if (typeid(Arg).hash_code() == type_hash) {
                *res = f(proxy_cast<Arg>(op));
            }
        }

        template<typename Arg, typename O, typename T>
            requires RttiAwareProxy<T>
        void try_invoke_void(O &&f, T &&op) {
            auto type_hash = proxy_typeid(op).hash_code();
            if (typeid(Arg).hash_code() == type_hash) {
                f(proxy_cast<Arg>(op));
            }
        }

        template<typename M, RttiAwareProxy P, std::size_t... Indices>
        typename M::Result visitImpl(M &&m, P &&ptr, std::index_sequence<Indices...>) {
            using PackArgs = typename M::CandidateArgs;
            if constexpr (std::is_same_v<typename M::Result, void>) {
                (try_invoke_void<std::tuple_element_t<Indices, PackArgs>>(m, std::forward<P>(ptr)), ...);
            } else {
                typename M::Result result;
                (try_invoke<std::tuple_element_t<Indices, PackArgs>>(&result, m, std::forward<P>(ptr)), ...);
                return result;
            }
        }
    } // namespace detail

    // TODO: keep const qualifier
    template<typename M, RttiAwareProxy P>
    typename M::Result visit(const M &m, P &&ptr) {
        constexpr auto size = std::tuple_size_v<typename M::CandidateArgs>;
        return detail::visitImpl(std::move(m), ptr, std::make_index_sequence<size>{});
    }

    template<typename... Fs>
    struct Matcher : Fs... {
        using Result = typename function_traits<PackIndex<0, Fs...>>::result_type;
        using CandidateArgs =
                std::tuple<std::remove_cvref_t<std::tuple_element_t<0, typename function_traits<Fs>::args_type>>...>;
        using Fs::operator()...;

        static_assert((std::is_same_v<Result, typename function_traits<Fs>::result_type> && ...),
                      "Result type mismatch");
    };

    template<typename... Fs>
    Matcher(Fs...) -> Matcher<Fs...>;

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_UTILITY_H_