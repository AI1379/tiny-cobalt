//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_STMTNODE_H_
#define TINY_COBALT_INCLUDE_AST_STMTNODE_H_

#include <memory>
#include <variant>

#define TINY_COBALT_AST_STMT_NODES(X, ...)                                                                             \
    X(If, __VA_ARGS__)                                                                                                 \
    X(While, __VA_ARGS__)                                                                                              \
    X(For, __VA_ARGS__)                                                                                                \
    X(Return, __VA_ARGS__)                                                                                             \
    X(Block, __VA_ARGS__)                                                                                              \
    X(Break, __VA_ARGS__)                                                                                              \
    X(Continue, __VA_ARGS__)                                                                                           \
    X(VariableDef, __VA_ARGS__)                                                                                        \
    X(FuncDef, __VA_ARGS__)                                                                                            \
    X(StructDef, __VA_ARGS__)                                                                                          \
    X(AliasDef, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_STMT_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_STMT_NODES(REG_STMT_NODE)

#undef REG_STMT_NODE

// #define REG_STMT_NODE(Name, Suffix) Name##Suffix,

//     using StmtNode = std::variant<TINY_COBALT_AST_STMT_NODES(REG_STMT_NODE, Node) std::monostate>;
//     using StmtNodePtr = std::variant<TINY_COBALT_AST_STMT_NODES(REG_STMT_NODE, Ptr) std::nullptr_t>;

// #undef REG_STMT_NODE

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_STMTNODE_H_