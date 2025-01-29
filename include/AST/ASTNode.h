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
#include "Common/Generator.h"
#include "Common/JSON.h"


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

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;


    inline std::ostream &operator<<(std::ostream &os, const ASTNodePtr &node) { return os << node->toJSON(); }

    template<typename T>
        requires ASTNodePtrConcept<T>
    inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &nodes) {
        os << "[";
        for (const auto &node: nodes)
            os << node << ",";
        os << "]";
        return os;
    }

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_