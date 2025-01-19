//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_STMTNODE_H_
#define TINY_COBALT_INCLUDE_AST_STMTNODE_H_

#include <memory>
#include <proxy.h>
#include "AST/ASTNode.h"

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
    X(AliasDef, __VA_ARGS__)                                                                                           \
    X(ExprStmt, __VA_ARGS__)                                                                                           \
    X(EmptyStmt, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_STMT_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_STMT_NODES(REG_STMT_NODE)

#undef REG_STMT_NODE

    // TODO: flag for stmt nodes;
    PRO_DEF_MEM_DISPATCH(MemStmtFlag, stmtFlag);

    struct StmtNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTNodeProxy, true> // NOLINT
          ::add_convention<MemStmtFlag, void()> // NOLINT
          ::build {};

    template<typename T>
    concept StmtNodeConcept = pro::proxiable<T *, StmtNodeProxy>;
    template<typename T>
    concept StmtNodePtrConcept = pro::proxiable<T, StmtNodeProxy>;
    using StmtNodePtr = pro::proxy<StmtNodeProxy>;

    static_assert(ASTNodePtrConcept<StmtNodePtr>, "StmtNodePtr is not an ASTNodePtr");

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_STMTNODE_H_