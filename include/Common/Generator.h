//
// Created by Renatus Madrigal on 12/15/2024
//

#ifndef TINY_COBALT_INCLUDE_COMMON_GENERATOR_H_
#define TINY_COBALT_INCLUDE_COMMON_GENERATOR_H_

#include <cassert>
#include <concepts>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iterator>
#include <memory>
#include <memory_resource>
#include <new>
#include <ranges>
#include <type_traits>
#include <utility>
#include <variant>

#include "Common/Range.h"

namespace TinyCobalt::Utility {
    // This is a implementation of std::generator in C++20 because libc++ does not support it yet.
    // This implementation is based on libstdc++'s implementation.

    template<typename Ref, typename Val = void, typename Alloc = void>
    class Generator;

    namespace detail {

        template<typename Ref, typename Val>
        using ReferenceType = std::conditional_t<std::is_void_v<Val>, Ref &&, Ref>;

        template<typename Ref>
        using YieldType = std::conditional_t<std::is_reference_v<Ref>, Ref, const Ref &>;

        template<typename Ref, typename Val>
        using Yield2Type = YieldType<ReferenceType<Ref, Val>>;

        template<typename>
        constexpr bool is_generator = false;
        template<typename Ref, typename Val, typename Alloc>
        constexpr bool is_generator<Generator<Ref, Val, Alloc>> = true;

        template<typename Yielded>
        class PromiseErased {
            static_assert(std::is_reference_v<Yielded>);

            using YieldedDeref = std::remove_reference_t<Yielded>;
            using YieldedDecvref = std::remove_cvref_t<Yielded>;
            using ValuePtr = std::add_pointer_t<Yielded>;
            using CoroHandle = std::coroutine_handle<PromiseErased>;

            template<typename, typename, typename>
            friend class Utility::Generator;

            template<typename Gen>
            struct RecursiveAwaiter;
            template<typename>
            friend struct RecursiveAwaiter;
            struct CopyAwaiter;
            struct SubyieldState;
            struct FinalAwaiter;

        public:
            std::suspend_always initial_suspend() noexcept { return {}; }
            std::suspend_always yield_value(Yielded value) noexcept {
                bottom_value() = std::addressof(value);
                return {};
            }

            auto yield_value(const YieldedDeref &value) noexcept(
                    std::is_nothrow_constructible_v<YieldedDeref, const YieldedDeref &>)
                requires(std::is_rvalue_reference_v<Yielded> &&
                         std::constructible_from<YieldedDecvref, const YieldedDeref &>)
            {
                return CopyAwaiter(value, bottom_value());
            }

            template<typename R2, typename V2, typename A2, typename U2>
                requires std::same_as<Yield2Type<R2, V2>, Yielded>
            auto yield_value(Ranges::ElementsOf<Generator<R2, V2, A2> &&, U2> r) noexcept {
                return RecursiveAwaiter{std::move(r.range)};
            }

            template<std::ranges::input_range R, typename Alloc>
                requires std::convertible_to<std::ranges::range_reference_t<R>, Yielded>
            auto yield_value(Ranges::ElementsOf<R, Alloc> &&r) noexcept {
                auto n = [](std::allocator_arg_t, Alloc, std::ranges::iterator_t<R> i,
                            std::ranges::sentinel_t<R> s) -> Generator<Yielded, std::ranges::range_value_t<R>, Alloc> {
                    for (; i != s; ++i)
                        co_yield static_cast<Yielded>(*i);
                };
                return yield_value(Ranges::ElementsOf(
                        n(std::allocator_arg, r.allocator, std::ranges::begin(r.range), std::ranges::end(r.range))));
            }

            FinalAwaiter final_suspend() noexcept { return {}; }

            void unhandled_exception() {
                if (nest.is_bottom())
                    throw;
                else
                    this->except = std::current_exception();
            }

            void await_transform() = delete;
            void return_void() const noexcept {}

        private:
            ValuePtr &bottom_value() noexcept { return nest.bottom_value(*this); }
            ValuePtr &value() noexcept { return nest.value(*this); }
            SubyieldState nest;
            std::exception_ptr except;
        };

        template<typename Yielded>
        struct PromiseErased<Yielded>::SubyieldState {
            struct Frame {
                CoroHandle bottom;
                CoroHandle parent;
            };

            struct BottomFrame {
                CoroHandle top;
                ValuePtr value = nullptr;
            };

            std::variant<BottomFrame, Frame> stack;

            bool is_bottom() const noexcept { return std::holds_alternative<BottomFrame>(stack); }

            CoroHandle &top() noexcept {
                if (auto f = std::get_if<Frame>(&this->stack))
                    return f->bottom.promise().nest.top();
                auto bf = std::get_if<BottomFrame>(&this->stack);
                assert(bf);
                return bf->top;
            }

            void push(CoroHandle cur, CoroHandle subyield) noexcept {
                assert(&cur.promise().nest == this);
                assert(this->top() == cur);

                subyield.promise().nest.jump_in(cur, subyield);
            }

            std::coroutine_handle<> pop() noexcept {
                if (auto f = std::get_if<Frame>(&this->stack)) {
                    // We are not a bottom coroutine.
                    // Restore the parent to the top and resume.
                    auto p = this->top() = f->parent;
                    return p;
                } else {
                    // Otherwise, there is nothing to resume.
                    return std::noop_coroutine();
                }
            }

            void jump_in(CoroHandle rest, CoroHandle new_top) noexcept {
                assert(&new_top.promise().nest == this);
                assert(this->is_bottom());
                assert(!this->top());

                auto &rn = rest.promise().nest;
                rn.top() = new_top;

                auto btm = rest;
                if (auto f = std::get_if<Frame>(&rn.stack)) {
                    btm = f->bottom;
                }
                this->stack = Frame{.bottom = btm, .parent = rest};
            }

            ValuePtr &bottom_value(PromiseErased &cur) noexcept {
                assert(&cur.nest == this);
                if (auto bf = std::get_if<BottomFrame>(&this->stack))
                    return bf->value;
                auto f = std::get_if<Frame>(&this->stack);
                assert(f);
                auto &p = f->bottom.promise();
                return p.nest.bottom_value(p);
            }

            ValuePtr &value(PromiseErased &cur) noexcept {
                assert(&cur.nest == this);
                auto bf = std::get_if<BottomFrame>(&this->stack);
                assert(bf);
                return bf->value;
            }
        };

        template<typename Yielded>
        struct PromiseErased<Yielded>::FinalAwaiter {

            bool await_ready() const noexcept { return false; }

            template<typename Promise>
            auto await_suspend(std::coroutine_handle<Promise> h) noexcept {
                return h.promise().nest.pop();
            }

            void await_resume() const noexcept {}
        };

        template<typename Yielded>
        struct PromiseErased<Yielded>::CopyAwaiter {
            YieldedDecvref value;
            ValuePtr &bottom_value;

            bool await_ready() const noexcept { return false; }

            template<typename Promise>
            void await_suspend(std::coroutine_handle<Promise>) noexcept {
                bottom_value = std::addressof(value);
            }

            void await_resume() const noexcept {}
        };

        template<typename Yielded>
        template<typename Gen>
        struct PromiseErased<Yielded>::RecursiveAwaiter {
            Gen gen;
            static_assert(is_generator<Gen>);
            static_assert(std::same_as<typename Gen::yielded, Yielded>);

            RecursiveAwaiter(Gen gen) noexcept : gen(std::move(gen)) { this->gen.mark_as_started(); }

            bool await_ready() const noexcept { return false; }

            template<typename Promise>
            auto await_suspend(std::coroutine_handle<Promise> h) noexcept {
                auto c = CoroHandle::from_address(h.address());
                auto t = CoroHandle::from_address(this->gen.coro.address());
                h.promise().nest.push(c, t);
                return t;
            }

            void await_resume() const noexcept {
                if (auto e = gen.coro.promise().except)
                    std::rethrow_exception(e);
            }
        };

        struct AllocBlock {
            // TODO: alignas(__STDCPP_DEFAULT_NEW_ALIGNMENT__)
            alignas(16UL) char data[16UL];

            static auto cnt(std::size_t sz) noexcept { return (sz + sizeof(AllocBlock) - 1) / sizeof(AllocBlock); }
        };

        template<typename All>
        concept StatelessAlloc =
                (std::allocator_traits<All>::is_always_equal::value && std::default_initializable<All>);

        template<typename Alloc>
        class PromiseAlloc {
            using AllocTraits = std::allocator_traits<Alloc>;
            using Rebound = typename AllocTraits::template rebind_alloc<AllocBlock>;
            using ReboundAllocTraits = typename AllocTraits::template rebind_traits<AllocBlock>;

            static_assert(std::is_pointer_v<typename ReboundAllocTraits::pointer>,
                          "Must use allocators for true pointers with generators");

            static auto alloc_address(std::uintptr_t fn, std::uintptr_t fnsiz) noexcept {
                auto an = fn + fnsiz;
                auto ba = alignof(Rebound);
                return reinterpret_cast<Rebound *>(((an + ba - 1) / ba) * ba);
            }

            static auto alloc_size(std::size_t sz) noexcept {
                // Comments from libstdc++-v3:
                // Our desired layout is placing the coroutine frame, then pad out to
                // align, then place the allocator.  The total size of that is the
                // size of the coroutine frame, plus up to alignof(Rebound) bytes, plus the size
                // of the allocator.
                return sz * alignof(Rebound) + sizeof(Rebound);
            }

            static void *allocate(Rebound b, std::size_t csz) {
                if constexpr (StatelessAlloc<Rebound>) {
                    return b.allocate(AllocBlock::cnt(csz));
                } else {
                    auto nsz = AllocBlock::cnt(alloc_size(csz));
                    auto f = b.allocate(nsz);
                    auto fn = reinterpret_cast<std::uintptr_t>(f);
                    auto an = alloc_address(fn, csz);
                    ::new (an) Rebound(std::move(b));
                    return f;
                }
            }

        public:
            void *operator new(std::size_t sz)
                requires std::default_initializable<Rebound>
            {
                return allocate({}, sz);
            }

            template<typename Na, typename... Args>
            void *operator new(std::size_t sz, std::allocator_arg_t, const Na &a, const Args &...)
                requires std::convertible_to<const Na &, Alloc>
            {
                return allocate(static_cast<Rebound>(static_cast<Alloc>(a)), sz);
            }

            template<typename This, typename Na, typename... Args>
            void *operator new(std::size_t sz, const This &, std::allocator_arg_t, const Na &a, const Args &...)
                requires std::convertible_to<const Na &, Alloc>
            {
                return allocate(static_cast<Rebound>(static_cast<Alloc>(a)), sz);
            }

            void operator delete(void *p, std::size_t sz) {
                if constexpr (StatelessAlloc<Rebound>) {
                    Rebound b;
                    return b.deallocate(reinterpret_cast<AllocBlock *>(p), AllocBlock::cnt(sz));
                } else {
                    auto nsz = AllocBlock::cnt(alloc_size(sz));
                    auto fn = reinterpret_cast<std::uintptr_t>(p);
                    auto an = alloc_address(fn, sz);
                    Rebound b(std::move(*an));
                    an->~Rebound();
                    b.deallocate(reinterpret_cast<AllocBlock *>(p), nsz);
                }
            }
        };

        template<>
        class PromiseAlloc<void> {
            using DeallocFunc = void (*)(void *, std::size_t);
            static auto dealloc_address(std::uintptr_t fn, std::uintptr_t fsz) noexcept {
                auto an = fn + fsz;
                auto ba = alignof(DeallocFunc);
                auto aligned = ((an + ba - 1) / ba) * ba;
                return reinterpret_cast<DeallocFunc *>(aligned);
            }

            template<typename Rebound>
            static auto alloc_address(std::uintptr_t fn, std::uintptr_t fsz) noexcept
                requires(!StatelessAlloc<Rebound>)
            {
                auto ba = alignof(Rebound);
                auto da = dealloc_address(fn, fsz);
                auto aan = reinterpret_cast<std::uintptr_t>(da);
                aan += sizeof(DeallocFunc);
                auto aligned = ((aan + ba - 1) / ba) * ba;
                return reinterpret_cast<Rebound *>(aligned);
            }

            template<typename Rebound>
            static auto alloc_size(std::size_t csz) noexcept {
                std::size_t aa = 0;
                std::size_t as = 0;
                if constexpr (!std::same_as<Rebound, void>) {
                    aa = alignof(Rebound);
                    as = sizeof(Rebound);
                }
                auto ba = aa + alignof(DeallocFunc);
                return csz + ba + as + sizeof(DeallocFunc);
            }

            template<typename Rebound>
            static void deallocate(void *p, std::size_t csz) {
                auto asz = alloc_size<Rebound>(csz);
                auto new_block = AllocBlock::cnt(asz);
                if constexpr (StatelessAlloc<Rebound>) {
                    Rebound b;
                    b.deallocate(reinterpret_cast<AllocBlock *>(p), new_block);
                } else {
                    auto fn = reinterpret_cast<std::uintptr_t>(p);
                    auto an = alloc_address<Rebound>(fn, csz);
                    Rebound b(std::move(*an));
                    an->~Rebound();
                    b.deallocate(reinterpret_cast<AllocBlock *>(p), new_block);
                }
            }

            template<typename Na>
            static void *allocate(const Na &na, std::size_t csz) {
                using Rebound = typename std::allocator_traits<Na>::template rebind_alloc<AllocBlock>;
                using ReboundAllocTraits = typename std::allocator_traits<Na>::template rebind_traits<AllocBlock>;

                static_assert(std::is_pointer_v<typename ReboundAllocTraits::pointer>,
                              "Must use allocators for true pointers with generators");

                DeallocFunc d = &deallocate<Rebound>;
                auto b = static_cast<Rebound>(na);
                auto asz = alloc_size<Rebound>(csz);
                auto new_block = AllocBlock::cnt(asz);
                void *p = b.allocate(new_block);
                auto pn = reinterpret_cast<std::uintptr_t>(p);
                *dealloc_address(pn, csz) = d;
                if constexpr (!StatelessAlloc<Rebound>) {
                    auto an = alloc_address<Rebound>(pn, csz);
                    ::new (an) Rebound(std::move(b));
                }
                return p;
            }

        public:
            void *operator new(std::size_t sz) {
                auto nsz = alloc_size<void>(sz);
                DeallocFunc d = [](void *ptr, std::size_t sz) {
                    // TODO: Check MSVC STL's implementation
                    ::operator delete(ptr, static_cast<size_t>(alloc_size<void>(sz)));
                };
                auto p = ::operator new(nsz);
                auto pn = reinterpret_cast<std::uintptr_t>(p);
                *dealloc_address(pn, sz) = d;
                return p;
            }

            template<typename Na, typename... Args>
            void *operator new(std::size_t sz, std::allocator_arg_t, const Na &a, const Args &...) {
                return allocate(a, sz);
            }

            template<typename This, typename Na, typename... Args>
            void *operator new(std::size_t sz, const This &, std::allocator_arg_t, const Na &a, const Args &...) {
                return allocate(a, sz);
            }

            void operator delete(void *p, std::size_t sz) {
                auto pn = reinterpret_cast<std::uintptr_t>(p);
                auto d = *dealloc_address(pn, sz);
                d(p, sz);
            }
        };

        template<typename T>
        concept CVUnqualifiedObject = std::is_object_v<T> && std::same_as<T, std::remove_cv_t<T>>;

    } // namespace detail

    template<typename Ref, typename Val, typename Alloc>
    class Generator : public std::ranges::view_interface<Generator<Ref, Val, Alloc>> {
        using Value = std::conditional_t<std::is_void_v<Val>, std::remove_cvref_t<Ref>, Val>;
        static_assert(detail::CVUnqualifiedObject<Value>, "Generator value must be a cv-unqualified object type");
        using Reference = detail::ReferenceType<Ref, Val>;
        static_assert(std::is_reference_v<Reference> ||
                              (detail::CVUnqualifiedObject<Reference> && std::copy_constructible<Reference>),
                      "Generator reference type must be either a cv-unqualified "
                      "object type that is trivially constructible or a "
                      "reference type");

        using RReference =
                std::conditional_t<std::is_reference_v<Reference>, std::remove_reference_t<Reference> &&, Reference>;

        static_assert(std::common_reference_with<Reference &&, Value &&>);
        static_assert(std::common_reference_with<Reference &&, RReference &&>);
        static_assert(std::common_reference_with<RReference &&, const Value &>);

        using Yielded = detail::YieldType<Reference>;
        using ErasedPromise = detail::PromiseErased<Yielded>;

        struct Iterator;

        friend ErasedPromise;
        friend struct ErasedPromise::SubyieldState;

    public:
        using yielded = Yielded;
        struct promise_type : ErasedPromise, detail::PromiseAlloc<Alloc> {
            Generator get_return_object() noexcept {
                return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
            }
        };

        Generator(const Generator &) = delete;

        Generator(Generator &&other) noexcept :
            coro(std::exchange(other.coro, nullptr)), began(std::exchange(other.began, false)) {}

        ~Generator() {
            if (auto &c = this->coro)
                c.destroy();
        }

        Generator &operator=(const Generator &) = delete;

        Generator &operator=(Generator other) noexcept {
            std::swap(this->coro, other.coro);
            std::swap(this->began, other.began);
        }

        Iterator begin() {
            this->mark_as_started();
            auto h = CoroHandle::from_promise(coro.promise());
            h.promise().nest.top() = h;
            return {h};
        }

        std::default_sentinel_t end() const noexcept { return std::default_sentinel; }

    private:
        using CoroHandle = std::coroutine_handle<ErasedPromise>;

        Generator(std::coroutine_handle<promise_type> coro) noexcept : coro{std::move(coro)} {}
        void mark_as_started() noexcept {
            assert(!this->began);
            this->began = true;
        }

        std::coroutine_handle<promise_type> coro;
        bool began = false;
    };

    template<typename Ref, typename Val, typename Alloc>
    struct Generator<Ref, Val, Alloc>::Iterator {
        using value_type = Value;
        using difference_type = std::ptrdiff_t;

        friend bool operator==(const Iterator &it, std::default_sentinel_t) noexcept { return it.coro.done(); }

        friend class Generator;

        Iterator(Iterator &&o) noexcept : coro(std::exchange(o.coro, {})) {}

        Iterator &operator=(Iterator &&o) noexcept {
            this->coro = std::exchange(o.coro, {});
            return *this;
        }

        Iterator &operator++() {
            next();
            return *this;
        }

        void operator++(int) { this->operator++(); }

        Reference operator*() const noexcept(std::is_nothrow_move_constructible_v<Reference>) {
            auto &p = this->coro.promise();
            return static_cast<Reference>(*p.value());
        }

    private:
        friend class Generator;
        Iterator(CoroHandle g) : coro{g} { this->next(); }

        void next() {
            auto &t = this->coro.promise().nest.top();
            t.resume();
        }

        CoroHandle coro;
    };

    namespace pmr {
        template<typename Ref, typename Val = void>
        using Generator = Generator<Ref, Val, std::pmr::polymorphic_allocator<std::byte>>;

        template<typename Ref, typename Val = void>
        using generator = Generator<Ref, Val>;
    } // namespace pmr

    template<typename Ref, typename Val = void, typename Alloc = void>
    using generator = Generator<Ref, Val, Alloc>;

} // namespace TinyCobalt::Utility

#endif // TINY_COBALT_INCLUDE_COMMON_GENERATOR_H_