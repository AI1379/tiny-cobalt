//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cstddef>
#include <proxy.h>
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Generator.h"
#include "Common/Utility.h"


#define TINY_COBALT_AST_NODES(X, ...)                                                                                  \
    TINY_COBALT_AST_EXPR_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_STMT_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_TYPE_NODES(X, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_NODE_EQ(Name, ...) bool operator==(const Name##Ptr &, const Name##Ptr &);

    TINY_COBALT_AST_EXPR_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_STMT_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_TYPE_NODES(REG_NODE_EQ)
    REG_NODE_EQ(ExprNode, )
    REG_NODE_EQ(StmtNode, )
    REG_NODE_EQ(TypeNode, )

#undef REG_NODE_EQ

    using ASTNode = Utility::UnionedVariant<ExprNode, StmtNode, TypeNode>;
    using ASTNodePtr = Utility::UnionedVariant<ExprNodePtr, StmtNodePtr, TypeNodePtr>;

#define REG_NODE_COMCEPT(Name, Suffix)

    template<typename T>
    concept ASTNodeConcept = Utility::IsVariantMember<T, ASTNode> || std::is_same_v<T, ExprNode> ||
                             std::is_same_v<T, StmtNode> || std::is_same_v<T, TypeNode>;

    template<typename T>
    concept ASTNodePtrConcept = Utility::IsVariantMember<T, ASTNodePtr> || std::is_same_v<T, ExprNodePtr> ||
                                std::is_same_v<T, StmtNodePtr> || std::is_same_v<T, TypeNodePtr>;

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);

    struct TraverseableProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<TraverseableProxy>>()> // NOLINT
          ::build {};

    template<typename T>
    concept Traverseable = pro::proxiable<T *, TraverseableProxy>;

    using TraverseablePtr = pro::proxy<TraverseableProxy>;
    using TraverseableGen = Utility::Generator<TraverseablePtr>;

    template<typename VisitorImpl>
    class ASTVisitor {
        void visit(this VisitorImpl &self, TraverseablePtr node) {
            for (auto child: node->traverse()) {
                self.visit(child);
            }
        }
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_