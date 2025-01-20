//
// Created by Renatus Madrigal on 12/15/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_RANGE_H_
#define TINY_COBALT_INCLUDE_COMMON_RANGE_H_

#include <concepts>
#include <cstddef>
#include <memory>
#include <range/v3/view/zip.hpp>
#include <ranges>

namespace TinyCobalt {
    namespace Ranges {
        template<typename Range, typename T>
        concept ContainerCompatibleRange =
                std::ranges::input_range<Range> && std::convertible_to<std::ranges::range_reference_t<Range>, T>;

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

        namespace views {
#if __cpp_lib_ranges_zip >= 202110L
            using std::views::zip;
#else
            using ranges::views::zip;
#endif
        } // namespace views

    } // namespace Ranges

    namespace views = Ranges::views;
    namespace ranges = Ranges;

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_RANGE_H_