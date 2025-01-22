//
// Created by Renatus Madrigal on 01/20/2025
//

#ifndef TINY_COBALT_INCLUDE_COMMON_FLATMAP_H_
#define TINY_COBALT_INCLUDE_COMMON_FLATMAP_H_

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>
#include "Common/Concept.h"
#include "Common/Range.h"

#if __cpp_lib_flat_map >= 202207L
#include <flat_map>
#endif

namespace TinyCobalt::Common {

#if __cpp_lib_flat_map >= 202207L
    // If the compiler supports std::flat_map, use it.
    using std::flat_map;
    using std::flat_multimap;

#else
    // Otherwise, use the custom implementation.

    struct sorted_unique_t {
        explicit sorted_unique_t() = default;
    };
    inline constexpr sorted_unique_t sorted_unique{};

    struct sorted_equivalent_t {
        explicit sorted_equivalent_t() = default;
    };
    inline constexpr sorted_equivalent_t sorted_equivalent{};

    // Pre-declaration
    template<typename, typename, typename, Container, Container>
    class flat_map;

    template<typename, typename, typename, Container, Container>
    class flat_multimap;

    // TODO: MACRO of [[no_unique_address]] for MSVC

    namespace detail {
        template<typename InputIter>
        concept HasInputIterCategory =
                std::is_convertible_v<typename std::iterator_traits<InputIter>::iterator_category,
                                      std::input_iterator_tag>;

        template<std::ranges::input_range _Range>
        using RangeKeyType = std::remove_const_t<typename std::ranges::range_value_t<_Range>::first_type>;
        template<std::ranges::input_range _Range>
        using RangeMappedType = typename std::ranges::range_value_t<_Range>::second_type;

        // TODO: figure what this concept is for
        template<typename Func>
        concept TransparentComparator = requires { typename Func::is_transparent; };

        template<typename Key, typename T, typename Comp, Container KeyContainer, Container MappedContainer,
                 bool IsMulti>
        class FlatMapImpl {
            // Assertions
            static_assert(std::is_same_v<Key, typename KeyContainer::value_type>, "Key type mismatch");
            static_assert(std::is_same_v<T, typename MappedContainer::value_type>, "Mapped type mismatch");

            static_assert(std::is_nothrow_swappable_v<KeyContainer>, "KeyContainer must be nothrow swappable");
            static_assert(std::is_nothrow_swappable_v<MappedContainer>, "MappedContainer must be nothrow swappable");

            template<bool isConst>
            struct Iterator;

            using Derived = std::conditional_t<IsMulti, flat_multimap<Key, T, Comp, KeyContainer, MappedContainer>,
                                               flat_map<Key, T, Comp, KeyContainer, MappedContainer>>;

        public:
            // STL Container Types
            using key_type = Key;
            using mapped_type = T;
            using value_type = std::pair<key_type, mapped_type>;
            using key_compare = Comp;
            using reference = std::pair<const key_type &, mapped_type &>;
            using const_reference = std::pair<const key_type &, const mapped_type &>;
            using size_type = size_t;
            using difference_type = std::ptrdiff_t;
            using iterator = Iterator<false>;
            using const_iterator = Iterator<true>;
            using reverse_iterator = std::reverse_iterator<iterator>;
            using const_reverse_iterator = std::reverse_iterator<const_iterator>;
            using key_container_type = KeyContainer;
            using mapped_container_type = MappedContainer;

            class value_compare {
            private:
                [[no_unique_address]] key_compare comp_;
                value_compare(key_compare comp) : comp_(std::move(comp)) {}

            public:
                bool operator()(const_reference lhs, const_reference rhs) const { return comp_(lhs.first, rhs.first); }
                friend FlatMapImpl;
            };

            struct containers {
                key_container_type keys;
                mapped_container_type values;
            };

        private:
            // ClearGuard to clear the containers when the scope ends
            struct ClearGuard {
                containers *cont_;
                ClearGuard(containers &cont) : cont_(std::addressof(cont)) {}
                ~ClearGuard() {
                    if (cont_) {
                        cont_->keys.clear();
                        cont_->values.clear();
                    }
                }
                void disable() { cont_ = nullptr; }
            };
            ClearGuard makeClearGuard() noexcept { return ClearGuard{this->cont_}; }
            using EmplaceResult = std::conditional_t<IsMulti, iterator, std::pair<iterator, bool>>;
            using Sorted = std::conditional<IsMulti, sorted_equivalent_t, sorted_unique_t>;

        public:
            // Constructors
            FlatMapImpl() : FlatMapImpl(key_compare()) {};

            explicit FlatMapImpl(const key_compare &comp) : cont_(), comp_(comp) {}

            FlatMapImpl(key_container_type keys, mapped_container_type values,
                        const key_compare &comp = key_compare()) :
                cont_{std::move(keys), std::move(values)}, comp_(comp) {
                assert(keys.size() == values.size());
                sortUnique_();
            }

            FlatMapImpl(Sorted, key_container_type keys, mapped_container_type values,
                        const key_compare &comp = key_compare()) :
                cont_{std::move(keys), std::move(values)}, comp_(comp) {
                assert(keys.size() == values.size());
                // TODO: Debug Assertion
                // assert(std::is_sorted(keys.begin(), keys.end(), comp_));
            }

            template<typename InputIterator>
                requires HasInputIterCategory<InputIterator>
            FlatMapImpl(InputIterator first, InputIterator last, const key_compare &comp = key_compare()) :
                cont_{}, comp_(comp) {
                insert(first, last);
            }

            template<typename InputIterator>
                requires HasInputIterCategory<InputIterator>
            FlatMapImpl(Sorted s, InputIterator first, InputIterator last, const key_compare &comp = key_compare()) :
                cont_{}, comp_(comp) {
                insert(s, first, last);
            }

            template<Ranges::ContainerCompatibleRange<value_type> Range>
            FlatMapImpl(ranges::from_range_t, Range &&range) :
                FlatMapImpl(ranges::from_range, std::forward<Range>(range), key_compare()) {}

            template<Ranges::ContainerCompatibleRange<value_type> Range>
            FlatMapImpl(Ranges::from_range_t, Range &&range, const key_compare &comp) : FlatMapImpl(comp) {
                insert_range(std::forward<Range>(range));
            }

            FlatMapImpl(std::initializer_list<value_type> il, const key_compare &comp = key_compare()) :
                FlatMapImpl(il.begin(), il.end(), comp) {}

            FlatMapImpl(Sorted s, std::initializer_list<value_type> il, const key_compare &comp = key_compare()) :
                FlatMapImpl(s, il.begin(), il.end(), comp) {}

            // Constructor with Allocators

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            explicit FlatMapImpl(const Alloc &alloc) : FlatMapImpl(key_compare(), alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(const key_compare &comp, const Alloc &alloc) :
                cont_{std::make_obj_using_allocator<key_container_type>(alloc),
                      std::make_obj_using_allocator<mapped_container_type>(alloc)},
                comp_(comp) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(const key_container_type &keys, const mapped_container_type &values, const Alloc &alloc) :
                FlatMapImpl(keys, values, key_compare(), alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(const key_container_type &keys, // NOLINT
                        const mapped_container_type &values, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) :
                cont_{std::make_obj_using_allocator<key_container_type>(alloc, keys),
                      std::make_obj_using_allocator<mapped_container_type>(alloc, values)},
                comp_(comp) {
                assert(keys.size() == values.size());
                sortUnique_();
            }

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        const key_container_type &keys, // NOLINT
                        const mapped_container_type &values, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(s, keys, values, key_compare(), alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        const key_container_type &keys, // NOLINT
                        const mapped_container_type &values, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) :
                cont_{std::make_obj_using_allocator<key_container_type>(alloc, keys),
                      std::make_obj_using_allocator<mapped_container_type>(alloc, values)},
                comp_(comp) {
                assert(keys.size() == values.size());
            }

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(const Derived &other, const Alloc &alloc) :
                cont_{std::make_obj_using_allocator<key_container_type>(alloc, other.cont_.keys),
                      std::make_obj_using_allocator<mapped_container_type>(alloc, other.cont_.values)},
                comp_(other.comp_) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Derived &&other, const Alloc &alloc) :
                cont_{std::make_obj_using_allocator<key_container_type>(alloc, std::move(other.cont_.keys)),
                      std::make_obj_using_allocator<mapped_container_type>(alloc, std::move(other.cont_.values))},
                comp_(other.comp_) {}

            template<HasInputIterCategory InputIter, AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(InputIter first, InputIter last, const Alloc &alloc) :
                FlatMapImpl(std::move(first), std::move(last), key_compare(), alloc) {}

            template<HasInputIterCategory InputIter, AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(InputIter first, // NOLINT
                        InputIter last, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(comp, alloc) {
                insert(first, last);
            }

            template<HasInputIterCategory InputIter, AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        InputIter first, // NOLINT
                        InputIter last, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(s, std::move(first), std::move(last), key_compare(), alloc) {}

            template<HasInputIterCategory InputIter, AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        InputIter first, // NOLINT
                        InputIter last, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(comp, alloc) {
                insert(s, first, last);
            }

            template<Ranges::ContainerCompatibleRange<value_type> Range, // NOLINT
                     AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(ranges::from_range_t, // NOLINT
                        Range &&range, // NOLINT
                        const Alloc &alloc) :
                FlatMapImpl(ranges::from_range, std::forward<Range>(range), key_compare(), alloc) {}

            template<Ranges::ContainerCompatibleRange<value_type> Range, // NOLINT
                     AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Ranges::from_range_t, // NOLINT
                        Range &&range, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(comp, alloc) {
                insert_range(std::forward<Range>(range));
            }

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(std::initializer_list<value_type> il, const Alloc &alloc) :
                FlatMapImpl(il, key_compare(), alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(std::initializer_list<value_type> il, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(il.begin(), il.end(), comp, alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        std::initializer_list<value_type> il, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(s, il.begin(), il.end(), key_compare(), alloc) {}

            template<AllocatorFor<key_container_type, mapped_container_type> Alloc>
            FlatMapImpl(Sorted s, // NOLINT
                        std::initializer_list<value_type> il, // NOLINT
                        const key_compare &comp, // NOLINT
                        const Alloc &alloc) : FlatMapImpl(s, il.begin(), il.end(), comp, alloc) {}

            auto &operator=(this auto &self, std::initializer_list<value_type> il) {
                self.clear();
                self.insert(il);
                return self;
            }

            // Iterators
            iterator begin() noexcept { return {this, cont_.keys.cbegin()}; }
            const_iterator begin() const noexcept { return {this, cont_.keys.cbegin()}; }
            iterator end() noexcept { return {this, cont_.keys.cend()}; }
            const_iterator end() const noexcept { return {this, cont_.keys.cend()}; }
            reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
            const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator(end()); }
            reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
            const_reverse_iterator rend() const noexcept { return const_reverse_iterator(begin()); }
            const_iterator cbegin() const noexcept { return {this, cont_.keys.cbegin()}; }
            const_iterator cend() const noexcept { return {this, cont_.keys.cend()}; }
            const_reverse_iterator crbegin() const noexcept { return const_reverse_iterator(cend()); }
            const_reverse_iterator crend() const noexcept { return const_reverse_iterator(cbegin()); }

            // Capacity
            [[nodiscard]] bool empty() const noexcept { return cont_.keys.empty(); }
            [[nodiscard]] size_type size() const noexcept { return cont_.keys.size(); }
            [[nodiscard]] size_type max_size() const noexcept {
                return std::min<size_type>(keys().max_size(), values().max_size());
            }

            // Observers
            [[nodiscard]] const key_container_type &keys() const noexcept { return cont_.keys; }
            [[nodiscard]] const mapped_container_type &values() const noexcept { return cont_.values; }
            // standard doesn't require them to be noexcept
            [[nodiscard]] key_compare key_comp() const { return key_compare(); }
            [[nodiscard]] value_compare value_comp() const { return value_compare(key_compare()); }

            // Element Access
            // These functions are not available in std::flat_multimap
            mapped_type &operator[](const key_type &x)
                requires(!IsMulti)
            {
                return operator[]<const key_type>(x);
            }
            mapped_type &operator[](key_type &&x)
                requires(!IsMulti)
            {
                return operator[]<key_type>(std::move(x));
            }
            template<typename Key2>
                requires((!IsMulti) &&
                         (std::same_as<std::remove_cvref_t<Key2>, key_type> || TransparentComparator<Comp>) )
            mapped_type &operator[](Key2 &&x) {
                return try_emplace(std::forward<Key2>(x)).first->second;
            }

            mapped_type &at(const key_type &x)
                requires(!IsMulti)
            {
                return at<key_type>(x);
            }

            const mapped_type &at(const key_type &x) const
                requires(!IsMulti)
            {
                return at<key_type>(x);
            }

            template<typename Key2>
                requires((!IsMulti) && (std::same_as<Key2, key_type> || TransparentComparator<Comp>) )
            mapped_type &at(const Key2 &x) {
                auto it = this->find(x);
                if (it == this->end()) {
                    throw std::out_of_range("flat_map::at");
                }
                return it->second;
            }

            template<typename Key2>
                requires((!IsMulti) && (std::same_as<Key2, key_type> || TransparentComparator<Comp>) )
            const mapped_type &at(const Key2 &x) const {
                auto it = this->find(x);
                if (it == this->end()) {
                    throw std::out_of_range("flat_map::at");
                }
                return it->second;
            }

        private:
            // Modifiers
            template<typename Key2, typename... Args>
            std::pair<iterator, bool> tryEmplace(std::optional<const_iterator> hint, Key2 &&key, Args &&...args) {
                typename key_container_type::iterator key_it;
                typename mapped_container_type::iterator value_it;
                int r = -1, s = -1; // two flags

                // TODO: simplify hint handling. maybe we should refer to the implementation of boost.
                if (hint.has_value() // NOLINT
                    && (hint == cbegin() || (r = !comp_(key, (*hint)[-1].first))) // k >= hint[-1]
                    && (hint == cend() || (s = !comp_((*hint)[0].first, key))) // k <= hint[0]
                ) {
                    key_it = cont_.keys.begin() + hint->index_;
                    if constexpr (!IsMulti) {
                        if (r == 1 && !comp_(key_it[-1], key))
                            return {iterator{this, key_it - 1}, false};
                    }
                } else {
                    auto first = cont_.keys.begin();
                    auto last = cont_.keys.end();
                    if (r == 1) { // k >= hint[-1]
                        first += hint->index_;
                    } else if (r == 0) { // k < hint[-1]
                        last = first + hint->index_;
                    }
                    if constexpr (IsMulti) {
                        if (s == 0) { // hint[0] < k
                            // Insert before the leftmost equivalent key.
                            key_it = std::lower_bound(first, last, key, comp_);
                        } else {
                            // Insert after the rightmost equivalent key.
                            key_it = std::upper_bound(std::make_reverse_iterator(last),
                                                      std::make_reverse_iterator(first), key, std::not_fn(comp_))
                                             .base();
                        }
                    } else {
                        key_it = std::lower_bound(cont_.keys.begin(), cont_.keys.end(), key, key_comp());
                    }
                }

                if constexpr (!IsMulti) {
                    if (key_it != cont_.keys.end() && !comp_(key, key_it[0])) {
                        return {iterator{this, key_it}, false};
                    }
                }
                auto guard = makeClearGuard();
                key_it = cont_.keys.insert(key_it, std::move(key));
                value_it = cont_.values.begin() + (key_it - cont_.keys.begin());
                cont_.values.emplace(value_it, std::forward<Args>(args)...);
                guard.disable();
                return {iterator{this, key_it}, true};
            }

        public:
            template<typename... Args>
                requires std::is_constructible_v<value_type, Args...>
            EmplaceResult emplace(Args &&...args) {
                value_type p(std::forward<Args>(args)...);
                auto r = tryEmplace(std::nullopt, std::move(p.first), std::move(p.second));
                if constexpr (IsMulti) {
                    return r.first;
                } else {
                    return r;
                }
            }

            template<typename... Args>
            iterator emplace_hint(const_iterator position, Args &&...args) {
                value_type p(std::forward<Args>(args)...);
                return tryEmplace(position, std::move(p.first), std::move(p.second)).first;
            }

            EmplaceResult insert(const value_type &x) { return emplace(x); }
            EmplaceResult insert(value_type &&x) { return emplace(std::move(x)); }
            iterator insert(const_iterator position, const value_type &x) { return emplace_hint(position, x); }
            iterator insert(const_iterator position, value_type &&x) { return emplace_hint(position, std::move(x)); }

            template<typename Arg>
                requires std::is_constructible_v<value_type, Arg>
            EmplaceResult insert(Arg &&arg) {
                return emplace(std::forward<Arg>(arg));
            }

            template<typename Arg>
                requires std::is_constructible_v<value_type, Arg>
            iterator insert(const_iterator position, Arg &&arg) {
                return emplace_hint(position, std::forward<Arg>(arg));
            }

            template<typename InputIter>
                requires HasInputIterCategory<InputIter>
            void insert(InputIter first, InputIter last) {
                // FIXME: GCC says that this fails its complexity requirements.
                auto guard = makeClearGuard();
                auto n = size();
                for (; first != last; ++first) {
                    value_type value = *first;
                    cont_.keys.emplace_back(std::move(value.first));
                    cont_.values.emplace_back(std::move(value.second));
                }
                sortUnique_();
                guard.disable();
            }

            template<typename InputIterator>
                requires HasInputIterCategory<InputIterator>
            void insert(Sorted, InputIterator first, InputIterator last) {
                insert(std::move(first), std::move(last));
            }

            template<typename Range>
                requires Ranges::ContainerCompatibleRange<Range, value_type>
            void insert_range(Range &&range) {
                insert(std::ranges::begin(range), std::ranges::end(range));
            }

            void insert(std::initializer_list<value_type> il) { insert(il.begin(), il.end()); }

            void insert(Sorted s, std::initializer_list<value_type> il) { insert(s, il.begin(), il.end()); }

            containers extract() && {
                auto guard = makeClearGuard();
                return {std::move(cont_.keys), std::move(cont_.values)};
            }

            void replace(key_container_type &&keys, mapped_container_type &&values) {
                assert(keys.size() == values.size());
                auto guard = makeClearGuard();
                cont_.keys = std::move(keys);
                cont_.values = std::move(values);
                guard.disable();
            }

            template<typename... Args>
                requires(!IsMulti && std::is_constructible_v<mapped_type, Args...>)
            std::pair<iterator, bool> try_emplace(const key_type &k, Args &&...args) {
                return tryEmplace(std::nullopt, k, std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires(!IsMulti && std::is_constructible_v<mapped_type, Args...>)
            std::pair<iterator, bool> try_emplace(const_iterator hint, const key_type &k, Args &&...args) {
                return tryEmplace(hint, k, std::forward<Args>(args)...);
            }

            template<typename Key2, typename... Args>
                requires(!IsMulti && // Only for flat_map
                         (TransparentComparator<Comp> && // transparent comparator
                          std::is_constructible_v<key_type, Key2> && // NOLINT
                          std::is_constructible_v<mapped_type, Args...> &&
                          (!std::is_constructible_v<Key2 &&, const_iterator>) &&
                          (!std::is_constructible_v<Key2 &&, iterator>) ))
            std::pair<iterator, bool> try_emplace(Key2 &&k, Args &&...args) {
                return tryEmplace(std::nullopt, std::forward<Key2>(k), std::forward<Args>(args)...);
            }

            template<typename... Args>
                requires(!IsMulti && std::is_constructible_v<mapped_type, Args...>)
            iterator try_emplace(const_iterator hint, key_type &&k, Args &&...args) {
                return tryEmplace(hint, std::move(k), std::forward<Args>(args)...).first;
            }

            template<typename... Args>
                requires(!IsMulti && std::is_constructible_v<mapped_type, Args...>)
            iterator try_emplace(const_iterator hint, const key_type &k, Args &&...args) {
                return tryEmplace(hint, k, std::forward<Args>(args)...).first;
            }

            template<typename Key2, typename... Args>
                requires(!IsMulti && TransparentComparator<Comp> && std::is_constructible_v<key_type, Key2> &&
                         std::is_constructible_v<mapped_type, Args...>)
            iterator try_emplace(const_iterator hint, Key2 &&k, Args &&...args) {
                return tryEmplace(hint, std::forward<Key2>(k), std::forward<Args>(args)...).first;
            }

            template<typename Mapped>
                requires(!IsMulti) && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            std::pair<iterator, bool> insert_or_assign(const key_type &k, Mapped &&obj) {
                return insert_or_assign<const key_type &, Mapped>(k, std::forward<Mapped>(obj));
            }

            template<typename Mapped>
                requires(!IsMulti) && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            std::pair<iterator, bool> insert_or_assign(key_type &&k, Mapped &&obj) {
                return insert_or_assign<key_type, Mapped>(std::move(k), std::forward<Mapped>(obj));
            }


            template<typename Key2, typename Mapped>
                requires(!IsMulti) &&
                        (std::same_as<std::remove_cvref_t<Key2>, key_type> || TransparentComparator<Comp>) &&
                        std::is_constructible_v<key_type, Key2> && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            std::pair<iterator, bool> insert_or_assign(Key2 &&k, Mapped &&obj) {
                auto r = tryEmplace(std::nullopt, std::forward<Key2>(k), std::forward<Mapped>(obj));
                if (!r.second) {
                    r.first->second = std::forward<Mapped>(obj);
                }
                return r;
            }

            template<typename Mapped>
                requires(!IsMulti) && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            iterator insert_or_assign(const_iterator hint, const key_type &k, Mapped &&obj) {
                return insert_or_assign<const key_type &, Mapped>(hint, k, std::forward<Mapped>(obj));
            }

            template<typename Mapped>
                requires(!IsMulti) && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            iterator insert_or_assign(const_iterator hint, key_type &&k, Mapped &&obj) {
                return insert_or_assign<key_type, Mapped>(hint, std::move(k), std::forward<Mapped>(obj));
            }

            template<typename Key2, typename Mapped>
                requires(!IsMulti) &&
                        (std::same_as<std::remove_cvref_t<Key2>, key_type> || TransparentComparator<Comp>) &&
                        std::is_constructible_v<key_type, Key2> && std::is_assignable_v<mapped_type &, Mapped> &&
                        std::is_constructible_v<mapped_type, Mapped>
            iterator insert_or_assign(const_iterator hint, Key2 &&k, Mapped &&obj) {
                auto r = tryEmplace(hint, std::forward<Key2>(k), std::forward<Mapped>(obj));
                if (!r.second) {
                    r.first->second = std::forward<Mapped>(obj);
                }
                return r.first;
            }


            iterator erase(iterator pos) { return erase(const_iterator{pos}); }

            iterator erase(const_iterator pos) {
                auto guard = makeClearGuard();
                auto idx = pos.index_;
                auto key_it = cont_.keys.erase(cont_.keys.begin() + idx);
                cont_.values.erase(cont_.values.begin() + idx);
                guard.disable();
                return iterator{this, key_it};
            }

            size_type erase(const key_type &x) { return erase<const key_type &>(x); }

            template<typename Key2>
                requires std::same_as<std::remove_cvref_t<Key2>, Key> ||
                         (TransparentComparator<Comp> && !std::is_convertible_v<Key2, iterator> &&
                          !std::is_convertible_v<Key2, const_iterator>)
            size_type erase(Key2 &&x) {
                auto [first, last] = this->equal_range(std::forward<Key2>(x));
                auto n = last - first;
                erase(first, last);
                return n;
            }

            iterator erase(const_iterator first, const_iterator last) {
                auto guard = makeClearGuard();
                auto it = cont_.keys.erase(cont_.keys.begin() + first.index_, cont_.keys.begin() + last.index_);
                cont_.values.erase(cont_.values.begin() + first.index_, cont_.values.begin() + last.index_);
                guard.disable();
                return iterator{this, it};
            }

            void swap(Derived &other) noexcept {
                std::ranges::swap(comp_, other.comp_);
                std::ranges::swap(cont_.keys, other.cont_.keys);
                std::ranges::swap(cont_.values, other.cont_.values);
            }

            void clear() noexcept {
                cont_.keys.clear();
                cont_.values.clear();
            }

            // Map operations
            [[nodiscard]] iterator find(const key_type &x) { return find<key_type>(x); }
            [[nodiscard]] const_iterator find(const key_type &x) const { return find<key_type>(x); }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] iterator find(const Key2 &x) {
                auto it = lower_bound(x);
                // TODO: it seems that the `!comp_(x, it->first)` is not necessary
                if (it != end() && !comp_(x, it->first)) {
                    return it;
                } else {
                    return end();
                }
            }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] const_iterator find(const Key2 &x) const {
                auto it = lower_bound(x);
                if (it != cend() && !comp_(x, it->first)) {
                    return it;
                } else {
                    return cend();
                }
            }

            [[nodiscard]] size_type count(const key_type &x) const { return count<key_type>(x); }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] size_type count(const Key2 &x) const {
                if constexpr (!IsMulti) {
                    return contains<Key2>(x);
                } else {
                    auto [first, last] = std::equal_range(x);
                    return last - first;
                }
            }

            [[nodiscard]] bool contains(const key_type &x) const { return contains<key_type>(x); }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] bool contains(const Key2 &x) const {
                return find(x) != cend();
            }

            [[nodiscard]] iterator lower_bound(const key_type &x) { return lower_bound<key_type>(x); }
            [[nodiscard]] const_iterator lower_bound(const key_type &x) const { return lower_bound<key_type>(x); }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] iterator lower_bound(const Key2 &x) {
                auto it = std::lower_bound(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {this, it};
            }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] const_iterator lower_bound(const Key2 &x) const {
                auto it = std::lower_bound(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {this, it};
            }

            [[nodiscard]] iterator upper_bound(const key_type &x) { return upper_bound<key_type>(x); }
            [[nodiscard]] const_iterator upper_bound(const key_type &x) const { return upper_bound<key_type>(x); }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] iterator upper_bound(const Key2 &x) {
                auto it = std::upper_bound(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {this, it};
            }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] const_iterator upper_bound(const Key2 &x) const {
                auto it = std::upper_bound(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {this, it};
            }

            [[nodiscard]] std::pair<iterator, iterator> equal_range(const key_type &x) {
                return equal_range<key_type>(x);
            }

            [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const key_type &x) const {
                return equal_range<key_type>(x);
            }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] std::pair<iterator, iterator> equal_range(const Key2 &x) {
                auto [first, last] = std::equal_range(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {{this, first}, {this, last}};
            }

            template<typename Key2>
                requires std::same_as<Key2, Key> || TransparentComparator<Comp>
            [[nodiscard]] std::pair<const_iterator, const_iterator> equal_range(const Key2 &x) const {
                auto [first, last] = std::equal_range(cont_.keys.begin(), cont_.keys.end(), x, comp_);
                return {{this, first}, {this, last}};
            }

            // Comparator
            [[nodiscard]] friend bool operator==(const Derived &x, const Derived &y) {
                return std::equal(x.begin(), x.end(), y.begin(), y.end());
            }

            template<typename Up = value_type>
            [[nodiscard]] friend Synth3way<Up> operator<=>(const Derived &x, const Derived &y) {
                return std::lexicographical_compare_three_way(x.begin(), x.end(), y.begin(), y.end(),
                                                              TinyCobalt::detail::kSynth3wayImpl);
            }

            friend void swap(Derived &x, Derived &y) noexcept { x.swap(y); }

            template<typename Predicate>
            friend size_type erase_if(Derived &c, Predicate pred) {
                auto guard = c.makeClearGuard();
                auto zv = Ranges::views::zip(c.cont_.keys, c.cont_.values);
                auto sr = std::ranges::remove_if(zv, pred);
                auto erased = sr.size();
                c.erase(c.end() - erased, c.end());
                guard.disable();
                return erased;
            }

        private:
            void unique_()
                requires(!IsMulti)
            {
                struct KeyEquiv {
                    KeyEquiv(key_compare c) : comp_(c) {}
                    bool operator()(const_reference x, const_reference y) const {
                        return !comp_(x.first, y.first) && !comp_(y.first, x.first);
                    }
                    [[no_unique_address]] key_compare comp_;
                };

                auto zv = Ranges::views::zip(cont_.keys, cont_.values);
                auto it = std::ranges::unique(zv, KeyEquiv(comp_)).begin();
                auto n = it - zv.begin();
                cont_.keys.erase(cont_.keys.begin() + n, cont_.keys.end());
                cont_.values.erase(cont_.values.begin() + n, cont_.values.end());
            }

            void sortUnique_() {
                auto zv = Ranges::views::zip(cont_.keys, cont_.values);
                std::ranges::sort(zv, value_comp());
                if constexpr (!IsMulti) {
                    unique_();
                }
            }

        private:
            containers cont_;
            [[no_unique_address]] key_compare comp_;
        };

        template<typename Key, typename T, typename Comp, Container KeyContainer, Container MappedContainer,
                 bool IsMulti>
        template<bool IsConst>
        class FlatMapImpl<Key, T, Comp, KeyContainer, MappedContainer, IsMulti>::Iterator {
            using size_type = typename FlatMapImpl::size_type;

        public:
            using iterator_category = std::input_iterator_tag;
            using iterator_concept = std::random_access_iterator_tag;
            using value_type = std::pair<const key_type, mapped_type>;
            using reference = std::pair<const key_type &, ConditionalConst<IsConst, mapped_type> &>;
            using difference_type = std::ptrdiff_t;

            Iterator() = default;

            Iterator(Iterator<!IsConst> it)
                requires IsConst
                : cont_(it.cont_), index_(it.index_) {}

            reference operator*() const noexcept {
                assert(index_ < cont_->keys.size());
                return {cont_->keys[index_], cont_->values[index_]};
            }

            struct pointer {
                reference p;
                const reference *operator->() const noexcept { return std::addressof(p); }
            };

            pointer operator->() const { return pointer{operator*()}; }

            reference operator[](difference_type n) const noexcept { return *(*this + n); }

            Iterator &operator++() noexcept {
                ++index_;
                return *this;
            }

            Iterator operator++(int) noexcept {
                auto tmp = *this;
                ++*this;
                return tmp;
            }

            Iterator &operator--() noexcept {
                --index_;
                return *this;
            }

            Iterator operator--(int) noexcept {
                auto tmp = *this;
                --*this;
                return tmp;
            }

            Iterator &operator+=(difference_type n) noexcept {
                index_ += n;
                return *this;
            }

            Iterator &operator-=(difference_type n) noexcept {
                index_ -= n;
                return *this;
            }

        private:
            friend FlatMapImpl;
            friend FlatMapImpl::Iterator<!IsConst>;

            Iterator(FlatMapImpl *fm, typename key_container_type::const_iterator it)
                requires(!IsConst)
                : cont_(std::addressof(fm->cont_)), index_(it - fm->keys().cbegin()) {}

            Iterator(const FlatMapImpl *fm, typename key_container_type::const_iterator it)
                requires IsConst
                : cont_(std::addressof(fm->cont_)), index_(it - fm->keys().cbegin()) {}

            friend Iterator operator+(Iterator it, difference_type n) noexcept {
                it += n;
                return it;
            }

            friend Iterator operator+(difference_type n, Iterator it) noexcept {
                it += n;
                return it;
            }

            friend Iterator operator-(Iterator it, difference_type n) noexcept {
                it -= n;
                return it;
            }

            friend difference_type operator-(Iterator lhs, Iterator rhs) noexcept {
                assert(lhs.cont_ == rhs.cont_);
                return lhs.index_ - rhs.index_;
            }

            friend bool operator==(const Iterator &lhs, const Iterator &rhs) noexcept {
                assert(lhs.cont_ == rhs.cont_);
                // Both iterators are end iterators.
                assert((lhs.index_ == size_t(-1)) == (rhs.index_ == size_t(-1)));
                return lhs.index_ == rhs.index_;
            }

            friend auto operator<=>(const Iterator &lhs, const Iterator &rhs) noexcept {
                assert(lhs.cont_ == rhs.cont_);
                assert((lhs.index_ == size_t(-1)) == (rhs.index_ == size_t(-1)));
                return lhs.index_ <=> rhs.index_;
            }

            [[no_unique_address]] ConditionalConst<IsConst, containers> *cont_ = nullptr;
            size_type index_ = -1;
        };
    } // namespace detail


    /**
     * An implementation of flat_map introduced in C++23 standard.
     */
    template<typename Key, // NOLINT
             typename T, // NOLINT
             typename Comp = std::less<Key>, // NOLINT
             Container KeyContainer = std::vector<Key>, // NOLINT
             Container MappedContainer = std::vector<T>>
    class flat_map : public detail::FlatMapImpl<Key, T, Comp, KeyContainer, MappedContainer, false> {
        using _Impl = detail::FlatMapImpl<Key, T, Comp, KeyContainer, MappedContainer, false>;
        friend _Impl;

    public:
        // FIXME: figure out why operator= cannot be exposed in the derived class even if with CRTP.

        // types
        using typename _Impl::const_iterator;
        using typename _Impl::const_reference;
        using typename _Impl::const_reverse_iterator;
        using typename _Impl::containers;
        using typename _Impl::difference_type;
        using typename _Impl::iterator;
        using typename _Impl::key_compare;
        using typename _Impl::key_container_type;
        using typename _Impl::key_type;
        using typename _Impl::mapped_container_type;
        using typename _Impl::mapped_type;
        using typename _Impl::reference;
        using typename _Impl::reverse_iterator;
        using typename _Impl::size_type;
        using typename _Impl::value_compare;
        using typename _Impl::value_type;

        // constructors
        using _Impl::_Impl;

        // iterators
        using _Impl::begin;
        using _Impl::end;
        using _Impl::rbegin;
        using _Impl::rend;

        using _Impl::cbegin;
        using _Impl::cend;
        using _Impl::crbegin;
        using _Impl::crend;

        // capacity
        using _Impl::empty;
        using _Impl::max_size;
        using _Impl::size;

        // element access
        using _Impl::at;
        using _Impl::operator[];

        // modifiers
        using _Impl::emplace;
        using _Impl::emplace_hint;
        using _Impl::insert;
        // using _Impl::insert_range;
        using _Impl::clear;
        using _Impl::erase;
        using _Impl::extract;
        using _Impl::replace;
        using _Impl::swap;

        using _Impl::insert_or_assign;
        using _Impl::try_emplace;

        // observers
        using _Impl::key_comp;
        using _Impl::keys;
        using _Impl::value_comp;
        using _Impl::values;

        // map operations
        using _Impl::contains;
        using _Impl::count;
        using _Impl::equal_range;
        using _Impl::find;
        using _Impl::lower_bound;
        using _Impl::upper_bound;
    };

    // Deduction guides
    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp = std::less<typename KeyContainer::value_type>>
    flat_map(KeyContainer, MappedContainer, Comp = Comp()) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, typename MappedContainer, AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_map(KeyContainer, MappedContainer, Alloc) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       std::less<typename KeyContainer::value_type>, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp, // NOLINT
             AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_map(KeyContainer, MappedContainer, Comp, Alloc) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp = std::less<typename KeyContainer::value_type>>
    flat_map(sorted_unique_t, KeyContainer, MappedContainer, Comp = Comp()) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, typename MappedContainer, AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_map(sorted_unique_t, KeyContainer, MappedContainer, Alloc) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       std::less<typename KeyContainer::value_type>, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp, // NOLINT
             AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_map(sorted_unique_t, KeyContainer, MappedContainer, Comp, Alloc) // NOLINT
            ->flat_map<typename KeyContainer::value_type, // NOLINT
                       typename MappedContainer::value_type, // NOLINT
                       Comp, KeyContainer, MappedContainer>;

    template<detail::HasInputIterCategory _InputIterator,
             NotAllocatorLike Comp = std::less<IteratorKeyType<_InputIterator>>>
    flat_map(_InputIterator, _InputIterator, Comp = Comp())
            -> flat_map<IteratorKeyType<_InputIterator>, IteratorValueType<_InputIterator>, Comp>;

    template<detail::HasInputIterCategory _InputIterator,
             NotAllocatorLike Comp = std::less<IteratorKeyType<_InputIterator>>>
    flat_map(sorted_unique_t, _InputIterator, _InputIterator, Comp = Comp())
            -> flat_map<IteratorKeyType<_InputIterator>, IteratorValueType<_InputIterator>, Comp>;

    template<std::ranges::input_range _Rg, // NOLINT
             NotAllocatorLike Comp = std::less<detail::RangeKeyType<_Rg>>, // NOLINT
             AllocatorLike Alloc = std::allocator<std::byte>>
    flat_map(ranges::from_range_t, _Rg &&, Comp = Comp(), Alloc = Alloc()) // NOLINT
            ->flat_map<detail::RangeKeyType<_Rg>, // NOLINT
                       detail::RangeMappedType<_Rg>, // NOLINT
                       Comp, // NOLINT
                       std::vector<detail::RangeKeyType<_Rg>, AllocRebind<Alloc, detail::RangeKeyType<_Rg>>>,
                       std::vector<detail::RangeMappedType<_Rg>, AllocRebind<Alloc, detail::RangeMappedType<_Rg>>>>;

    template<std::ranges::input_range _Rg, AllocatorLike Alloc>
    flat_map(ranges::from_range_t, _Rg &&, Alloc) // NOLINT
            ->flat_map<detail::RangeKeyType<_Rg>, // NOLINT
                       detail::RangeMappedType<_Rg>, // NOLINT
                       std::less<detail::RangeKeyType<_Rg>>, // NOLINT
                       std::vector<detail::RangeKeyType<_Rg>, AllocRebind<Alloc, detail::RangeKeyType<_Rg>>>,
                       std::vector<detail::RangeMappedType<_Rg>, AllocRebind<Alloc, detail::RangeMappedType<_Rg>>>>;

    template<typename Key, typename _Tp, NotAllocatorLike Comp = std::less<Key>>
    flat_map(std::initializer_list<std::pair<Key, _Tp>>, Comp = Comp()) -> flat_map<Key, _Tp, Comp>;

    template<typename Key, typename _Tp, NotAllocatorLike Comp = std::less<Key>>
    flat_map(sorted_unique_t, std::initializer_list<std::pair<Key, _Tp>>, Comp = Comp()) -> flat_map<Key, _Tp, Comp>;

    /**
     * An implementation of flat_multimap introduced in C++23 standard.
     */
    template<typename Key, // NOLINT
             typename T, // NOLINT
             typename Comp = std::less<Key>, // NOLINT
             Container KeyContainer = std::vector<Key>, // NOLINT
             Container MappedContainer = std::vector<T>>
    class flat_multimap : detail::FlatMapImpl<Key, T, Comp, KeyContainer, MappedContainer, true> {};


    // Deduction guides
    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp = std::less<typename KeyContainer::value_type>>
    flat_multimap(KeyContainer, MappedContainer, Comp = Comp()) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_multimap(KeyContainer, MappedContainer, Alloc) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            std::less<typename KeyContainer::value_type>, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp, // NOLINT
             AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_multimap(KeyContainer, MappedContainer, Comp, Alloc) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp = std::less<typename KeyContainer::value_type>>
    flat_multimap(sorted_equivalent_t, KeyContainer, MappedContainer, Comp = Comp()) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            Comp, KeyContainer, MappedContainer>;

    template<typename KeyContainer, typename MappedContainer, AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_multimap(sorted_equivalent_t, KeyContainer, MappedContainer, Alloc) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            std::less<typename KeyContainer::value_type>, KeyContainer, MappedContainer>;

    template<typename KeyContainer, // NOLINT
             typename MappedContainer, // NOLINT
             NotAllocatorLike Comp, // NOLINT
             AllocatorFor<KeyContainer, MappedContainer> Alloc>
    flat_multimap(sorted_equivalent_t, KeyContainer, MappedContainer, Comp, Alloc) // NOLINT
            ->flat_multimap<typename KeyContainer::value_type, // NOLINT
                            typename MappedContainer::value_type, // NOLINT
                            Comp, KeyContainer, MappedContainer>;

    template<detail::HasInputIterCategory _InputIterator,
             NotAllocatorLike Comp = std::less<IteratorKeyType<_InputIterator>>>
    flat_multimap(_InputIterator, _InputIterator, Comp = Comp())
            -> flat_multimap<IteratorKeyType<_InputIterator>, IteratorValueType<_InputIterator>, Comp>;

    template<detail::HasInputIterCategory _InputIterator,
             NotAllocatorLike Comp = std::less<IteratorKeyType<_InputIterator>>>
    flat_multimap(sorted_equivalent_t, _InputIterator, _InputIterator, Comp = Comp())
            -> flat_multimap<IteratorKeyType<_InputIterator>, IteratorValueType<_InputIterator>, Comp>;

    template<std::ranges::input_range _Rg, // NOLINT
             NotAllocatorLike Comp = std::less<detail::RangeKeyType<_Rg>>, // NOLINT
             AllocatorLike Alloc = std::allocator<std::byte>>
    flat_multimap(ranges::from_range_t, _Rg &&, Comp = Comp(), Alloc = Alloc()) // NOLINT
            ->flat_multimap<
                    detail::RangeKeyType<_Rg>, // NOLINT
                    detail::RangeMappedType<_Rg>, // NOLINT
                    Comp, // NOLINT
                    std::vector<detail::RangeKeyType<_Rg>, AllocRebind<Alloc, detail::RangeKeyType<_Rg>>>,
                    std::vector<detail::RangeMappedType<_Rg>, AllocRebind<Alloc, detail::RangeMappedType<_Rg>>>>;

    template<std::ranges::input_range _Rg, AllocatorLike Alloc>
    flat_multimap(ranges::from_range_t, _Rg &&, Alloc) // NOLINT
            ->flat_multimap<
                    detail::RangeKeyType<_Rg>, // NOLINT
                    detail::RangeMappedType<_Rg>, // NOLINT
                    std::less<detail::RangeKeyType<_Rg>>, // NOLINT
                    std::vector<detail::RangeKeyType<_Rg>, AllocRebind<Alloc, detail::RangeKeyType<_Rg>>>,
                    std::vector<detail::RangeMappedType<_Rg>, AllocRebind<Alloc, detail::RangeMappedType<_Rg>>>>;

    template<typename Key, typename _Tp, NotAllocatorLike Comp = std::less<Key>>
    flat_multimap(std::initializer_list<std::pair<Key, _Tp>>, Comp = Comp()) -> flat_multimap<Key, _Tp, Comp>;

    template<typename Key, typename _Tp, NotAllocatorLike Comp = std::less<Key>>
    flat_multimap(sorted_equivalent_t, std::initializer_list<std::pair<Key, _Tp>>, Comp = Comp())
            -> flat_multimap<Key, _Tp, Comp>;


#endif

} // namespace TinyCobalt::Common

#endif // TINY_COBALT_INCLUDE_COMMON_FLATMAP_H_