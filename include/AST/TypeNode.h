//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_TYPENODE_H_
#define TINY_COBALT_INCLUDE_AST_TYPENODE_H_

#include <memory>
#include <variant>

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

// #define REG_TYPE_NODE(Name, Suffix) Name##Suffix,

//     using TypeNode = std::variant<TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE, Node) std::monostate>;
//     using TypeNodePtr = std::variant<TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE, Ptr) std::nullptr_t>;

// #undef REG_TYPE_NODE

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_TYPENODE_H_