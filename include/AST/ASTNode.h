//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cstddef>
#include <proxy.h>
#include <utility>
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

    // using ASTNode = Utility::UnionedVariant<ExprNode, StmtNode, TypeNode>;
    // using ASTNodePtr = Utility::UnionedVariant<ExprNodePtr, StmtNodePtr, TypeNodePtr>;

    struct ASTVisitorProxy; // TODO: implement ASTVisitorProxy

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);

    class ASTNodeVisitorRefl {
    public:
        template<typename T>
        constexpr explicit ASTNodeVisitorRefl(std::in_place_type_t<T>, pro::proxy<ASTVisitorProxy> &visitor) {
            // down cast Proxy type to T and call visitor.visit
        }

        template<typename F, typename R>
        struct accessor {
            void visit(pro::proxy<ASTVisitorProxy> &visitor) const noexcept {
                const ASTNodeVisitorRefl &self = {pro::proxy_reflect<R>(pro::access_proxy<F>(*this)), visitor};
            }
        };
    };

    class RttiReflector {
    public:
        template<class T>
        constexpr explicit RttiReflector(std::in_place_type_t<T>) : type_(typeid(T)) {}

        template<class F, class R>
        struct accessor {
            const char *GetTypeName() const noexcept {
                const RttiReflector &self = pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
                return self.type_.name();
            }
        };

    private:
        const std::type_info &type_;
    };

    struct ASTNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_reflection<ASTNodeVisitorRefl> // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<ASTNodeProxy>>()> // NOLINT
          ::build {};

    template<typename T>
    concept IsASTNode = pro::proxiable<T *, ASTNodeProxy>;

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_