//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <proxy.h>
#include <utility>
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Generator.h"


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

    // using ASTNode = Utility::UnionedVariant<ExprNode, StmtNode, TypeNode>;
    // using ASTNodePtr = Utility::UnionedVariant<ExprNodePtr, StmtNodePtr, TypeNodePtr>;

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);

    struct ContainTypeRefl {
        template<typename T, typename U>
        constexpr explicit ContainTypeRefl(std::in_place_type_t<T>, U &&) : same_(std::is_same_v<T, U>) {}

        template<typename F, typename R>
        struct accessor {
            template<typename T>
            bool containType() const noexcept {
                const ContainTypeRefl &self = {pro::proxy_reflect<F>(pro::access_proxy<F>(*this)), std::declval<T>()};
                return self.same_;
            }
        };

    private:
        bool same_;
    };


    struct ASTNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_reflection<ContainTypeRefl> // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<ASTNodeProxy>>()> // NOLINT
          ::build {};

    template<typename T>
    concept ASTNodeConcept = pro::proxiable<T *, ASTNodeProxy>;

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_