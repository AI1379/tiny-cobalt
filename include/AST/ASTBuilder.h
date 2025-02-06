//
// Created by Renatus Madrigal on 02/06/2025
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTBUILDER_H_
#define TINY_COBALT_INCLUDE_AST_ASTBUILDER_H_

#include <concepts>
#include <memory>
#include <utility>
#include <vector>

namespace TinyCobalt::AST::Builder {
    template<typename T>
    struct Node {
        using ElementType = typename std::pointer_traits<T>::element_type;

        template<typename... Args>
            requires std::constructible_from<ElementType, Args...>
        explicit Node(Args &&...args) : node_(std::make_shared<ElementType>(std::forward<Args>(args)...)) {}

        T operator()() { return node_; }

    private:
        T node_;
    };

    template<typename P>
    struct Array {
        template<typename... Args>
        explicit Array(Args &&...args) : array_({args...}) {}

        std::vector<P> &&operator()() { return std::move(array_); }

    private:
        std::vector<P> array_;
    };
} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTBUILDER_H_