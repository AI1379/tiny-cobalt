//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"

#include <proxy.h>

#include <string>


namespace TinyCobalt::AST {

    PRO_DEF_FREE_DISPATCH(FreeToString, std::to_string, ToString);

    struct Stringable : pro::facade_builder ::add_convention<FreeToString, std::string()>::build {};

    static_assert(pro::proxiable<int *, Stringable>);
    static_assert(pro::proxiable<std::shared_ptr<double>, Stringable>);


    namespace detail {
#define REG_NODE_DUMP(Name, ...) void dumpImpl(const Name##Ptr &, std::ostream &os);

        TINY_COBALT_AST_EXPR_NODES(REG_NODE_DUMP)
        TINY_COBALT_AST_STMT_NODES(REG_NODE_DUMP)
        TINY_COBALT_AST_TYPE_NODES(REG_NODE_DUMP)
        REG_NODE_DUMP(ExprNode, )
        REG_NODE_DUMP(StmtNode, )
        REG_NODE_DUMP(TypeNode, )

#undef REG_NODE_DUMP
    } // namespace detail

#define REG_NODE_EQ(Name, ...) bool operator==(const Name##Ptr &, const Name##Ptr &);

    TINY_COBALT_AST_EXPR_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_STMT_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_TYPE_NODES(REG_NODE_EQ)
    REG_NODE_EQ(ExprNode, )
    REG_NODE_EQ(StmtNode, )
    REG_NODE_EQ(TypeNode, )

#undef REG_NODE_EQ

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_