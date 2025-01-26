//
// Created by Renatus Madrigal on 01/21/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_
#define TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_

#include <cassert>
#include <compare>
#include <concepts>
#include <memory>
#include <type_traits>
#include <utility>

namespace TinyCobalt {

    template<typename Alloc>
    concept AllocatorLike = requires(Alloc &alloc) {
        typename Alloc::value_type;
        alloc.deallocate(alloc.allocate(1u), 1u);
    };

    template<typename Alloc>
    concept NotAllocatorLike = !AllocatorLike<Alloc>;

    template<typename Alloc, typename... Args>
    concept AllocatorFor = (std::uses_allocator_v<Args, Alloc> && ...);

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

    template<typename T>
    struct is_tuple : std::false_type {};

    template<typename... Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type {};

    template<typename T>
    inline constexpr bool is_tuple_v = is_tuple<T>::value;

    template<typename T, typename R>
    inline constexpr bool invocable_tuple_v = false;

    template<typename T, typename... Ts>
    inline constexpr bool invocable_tuple_v<T, std::tuple<Ts...>> = requires(T t, Ts... ts) {
        { t(ts...) };
    };

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_