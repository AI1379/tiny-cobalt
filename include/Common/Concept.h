//
// Created by Renatus Madrigal on 01/21/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_
#define TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_

#include <cassert>
#include <compare>
#include <concepts>
#include <memory>
#include <utility>

namespace TinyCobalt {

    namespace detail {
        template<typename T>
        concept BooleanTestableImpl = std::convertible_to<T, bool>;

        template<typename T>
        concept BooleanTestable = BooleanTestableImpl<T> && requires(T &&t) {
            { !static_cast<T &&>(t) } -> BooleanTestableImpl;
        };

        inline constexpr struct Synth3wayImpl {
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
                    { t < u } -> BooleanTestable;
                    { u < t } -> BooleanTestable;
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
        } kSynth3wayImpl = {};

    } // namespace detail

    template<typename T, typename U = T>
    using Synth3way = decltype(detail::kSynth3wayImpl(std::declval<T &>(), std::declval<U &>()));

    template<typename Alloc>
    concept AllocatorLike = requires(Alloc &alloc) {
        typename Alloc::value_type;
        alloc.deallocate(alloc.allocate(1u), 1u);
    };

    template<typename Alloc>
    concept NotAllocatorLike = !AllocatorLike<Alloc>;

    template<typename Alloc, typename... Args>
    concept AllocatorFor = (std::uses_allocator_v<Args, Alloc> && ...);

    template<typename Alloc, typename T>
    using AllocRebind = std::allocator_traits<Alloc>::template rebind_alloc<T>;

    template<typename InputIter>
    using IteratorKeyType = std::remove_const_t<typename std::iterator_traits<InputIter>::value_type::first_type>;

    template<typename InputIter>
    using IteratorValueType = std::remove_const_t<typename std::iterator_traits<InputIter>::value_type::second_type>;

    template<bool IsConst, typename T>
    using ConditionalConst = std::conditional_t<IsConst, const T, T>;

    template<typename C>
    concept Container = requires(C cont) {
        typename C::value_type;
        typename C::reference;
        typename C::const_reference;
        typename C::iterator;
        typename C::const_iterator;
        typename C::size_type;
        typename C::difference_type;
        { cont.begin() } -> std::same_as<typename C::iterator>;
        { cont.end() } -> std::same_as<typename C::iterator>;
        { cont.cbegin() } -> std::same_as<typename C::const_iterator>;
        { cont.cend() } -> std::same_as<typename C::const_iterator>;
        { cont.size() } -> std::same_as<typename C::size_type>;
        { cont.max_size() } -> std::same_as<typename C::size_type>;
        { cont.empty() } -> std::same_as<bool>;
    };
} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_