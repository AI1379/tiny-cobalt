//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <proxy.h>
#include <type_traits>
#include <variant>
#include "Common/Generator.h"
#include "Common/JSON.h"
#include "Common/Utility.h"


#define TINY_COBALT_AST_NODES(X, ...)                                                                                  \
    TINY_COBALT_AST_EXPR_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_STMT_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_TYPE_NODES(X, __VA_ARGS__)                                                                         \
    X(ASTRoot, __VA_ARGS__)

namespace TinyCobalt::AST {

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);
    PRO_DEF_MEM_DISPATCH(MemThisPointer, thisPointer);

    struct ASTNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<ASTNodeProxy>>()> // NOLINT
          // FIXME: erased type information may lead to memory leaks
          ::add_convention<MemThisPointer, void *() const> // NOLINT
          // TODO: rewrite toJSON() using generator.
          ::add_facade<Common::ToJSONProxy> // NOLINT
          ::support_copy<pro::constraint_level::nontrivial> // NOLINT
          ::support_rtti // NOLINT
          ::support_direct_rtti // NOLINT
          ::build {};


    template<typename T>
    struct EnableThisPointer : public std::enable_shared_from_this<T> {
        void *thisPointer() const { return const_cast<void *>(reinterpret_cast<const void *>(this)); }
    };

    template<typename T>
    concept ASTNodeConcept = pro::proxiable<T *, ASTNodeProxy>;

    template<typename T>
    concept ASTNodePtrConcept = pro::proxiable<T, ASTNodeProxy> || std::is_convertible_v<T, pro::proxy<ASTNodeProxy>>;

    namespace detail {
        template<typename T>
        struct VariantASTNodePtrImpl : std::false_type {};

        template<typename... Ts>
        struct VariantASTNodePtrImpl<std::variant<Ts...>> {
            static constexpr bool value = (ASTNodePtrConcept<Ts> && ...);
        };
    } // namespace detail

    template<typename T>
    concept VariantASTNodePtrConcept = detail::VariantASTNodePtrImpl<T>::value;

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;


    inline std::ostream &operator<<(std::ostream &os, const ASTNodePtr &node) { return os << node->toJSON(); }

    template<typename T>
        requires is_variant_v<T>
    inline std::ostream &operator<<(std::ostream &os, const T &node) {
        std::visit([&os](const auto &node) { os << node; }, node);
        return os;
    }

    template<typename T>
        requires(ASTNodePtrConcept<T> || is_variant_v<T>)
    inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &nodes) {
        os << "[";
        for (const auto &node: nodes)
            os << node << ",";
        os << "]";
        return os;
    }

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_