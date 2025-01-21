//
// Created by Renatus Madrigal on 01/21/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_
#define TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_

#include <compare>
#include <concepts>

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
    using Synth3way = decltype(kSynth3wayImpl(std::declval<T &>(), std::declval<U &>()));

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