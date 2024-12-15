//
// Created by Renatus Madrigal on 12/15/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_RANGE_H_
#define TINY_COBALT_INCLUDE_COMMON_RANGE_H_

#include <cstddef>
#include <memory>
#include <ranges>

namespace TinyCobalt::Ranges {
    // This is an implementation of std::ranges::elements_of in C++20 based on the implementation of MSVC's STL
    template<std::ranges::range Rng, typename Alloc = std::allocator<std::byte>>
    struct ElementsOf {
        Rng range;
        Alloc allocator{};
    };
    template<typename Rng, typename Alloc = std::allocator<std::byte>>
    ElementsOf(Rng &&, Alloc = Alloc()) -> ElementsOf<Rng &&, Alloc>;

    template<typename Rng, typename Alloc = std::allocator<std::byte>>
    using elements_of = ElementsOf<Rng, Alloc>;
} // namespace TinyCobalt::Ranges

#endif // TINY_COBALT_INCLUDE_COMMON_RANGE_H_