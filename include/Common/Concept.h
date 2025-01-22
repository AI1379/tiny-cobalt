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
} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_CONCEPT_H_