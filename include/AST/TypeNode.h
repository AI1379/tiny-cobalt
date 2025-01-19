//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_TYPENODE_H_
#define TINY_COBALT_INCLUDE_AST_TYPENODE_H_

#include <memory>
#include <proxy.h>
#include "AST/ASTNode.h"

#define TINY_COBALT_AST_TYPE_NODES(X, ...)                                                                             \
    X(SimpleType, __VA_ARGS__)                                                                                         \
    X(FuncType, __VA_ARGS__)                                                                                           \
    X(ComplexType, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_TYPE_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE)

#undef REG_TYPE_NODE

    PRO_DEF_MEM_DISPATCH(MemConvertibleTo, convertibleTo);

    struct TypeNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTNodeProxy, true> // NOLINT
          ::add_convention<MemConvertibleTo, bool(const pro::proxy<TypeNodeProxy> &) const> // NOLINT
          ::build {};

    template<typename T>
    concept TypeNodeConcept = pro::proxiable<T *, TypeNodeProxy>;
    template<typename T>
    concept TypeNodePtrConcept = pro::proxiable<T, TypeNodeProxy>;
    using TypeNodePtr = pro::proxy<TypeNodeProxy>;

    static_assert(ASTNodePtrConcept<TypeNodePtr>, "TypeNodePtr is not an ASTNodePtr");

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_TYPENODE_H_