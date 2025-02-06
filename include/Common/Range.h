//
// Created by Renatus Madrigal on 12/15/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_RANGE_H_
#define TINY_COBALT_INCLUDE_COMMON_RANGE_H_

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#if __cpp_lib_ranges_zip < 202110L
#include <range/v3/view/zip.hpp>
#endif

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
            // FIXME: ranges::views::zip in range-v3 seems not to be exactly the same as std::views::zip
            // However the compiler error cannot be reproduced now.
            // In commit 180ce80455c152905572d5870c57093d881d8e3c the error might be reproduced.
            using ranges::views::zip;
#endif
        } // namespace views

#if __cpp_lib_containers_ranges >= 202202L
        using std::from_range;
        using std::from_range_t;
#else
        struct from_range_t {
            explicit from_range_t() = default;
        };
        inline constexpr from_range_t from_range{};
#endif

    } // namespace Ranges

    namespace views = Ranges::views;
    namespace ranges = Ranges;

} // namespace TinyCobalt

#endif // TINY_COBALT_INCLUDE_COMMON_RANGE_H_