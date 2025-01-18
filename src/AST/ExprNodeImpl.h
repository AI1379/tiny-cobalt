//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_
#define TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/TypeNode.h"

#include <proxy.h>
#include <string>
#include <vector>

namespace TinyCobalt::AST {
    // TODO: Compile-time evaluation
    // FIXME: Implement evalType()
    struct ConstExprNode : public EnableThisPointer<ConstExprNode> {
        const std::string value;
        const ConstExprType expr_type;
        explicit ConstExprNode(std::string value, ConstExprType expr_type) : value(value), expr_type(expr_type) {}
        ASTNodeGen traverse() { co_return; }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    struct VariableNode : public EnableThisPointer<VariableNode> {
        const std::string name;
        explicit VariableNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse() { co_return; }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    // TODO: support infix for binary function like Haskell
    struct BinaryNode : public EnableThisPointer<BinaryNode> {
        BinaryOp op;
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit BinaryNode(ExprNodePtr lhs, BinaryOp op, ExprNodePtr rhs) :
            op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        // BinaryNode(const BinaryNode &) = default;
        // BinaryNode &operator=(const BinaryNode &) = default;
        ASTNodeGen traverse() {
            co_yield lhs;
            co_yield rhs;
        }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    struct UnaryNode : public EnableThisPointer<UnaryNode> {
        const UnaryOp op;
        ExprNodePtr operand;
        explicit UnaryNode(UnaryOp op, ExprNodePtr operand) : op(std::move(op)), operand(std::move(operand)) {}
        ASTNodeGen traverse() { co_yield operand; }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    // Operators with multiple params, for example, operator[].
    // Because we allow operator overload, thus this support is necessary.
    struct MultiaryNode : public EnableThisPointer<MultiaryNode> {
        const MultiaryOp op;
        std::string object;
        std::vector<ExprNodePtr> operands;
        explicit MultiaryNode(MultiaryOp op, std::string obj, std::vector<ExprNodePtr> operands = {}) :
            op(op), object(std::move(obj)), operands(std::move(operands)) {}
        ASTNodeGen traverse() { co_return; }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    struct CastNode : public EnableThisPointer<CastNode> {
        const CastType op;
        TypeNodePtr type;
        ExprNodePtr operand;
        explicit CastNode(CastType op, TypeNodePtr type, ExprNodePtr operand) :
            op(op), type(std::move(type)), operand(std::move(operand)) {}
        ASTNodeGen traverse() {
            co_yield type;
            co_yield operand;
        }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

    // Three way conditional operator
    struct ConditionNode : public EnableThisPointer<ConditionNode> {
        ExprNodePtr condition;
        ExprNodePtr trueBranch;
        ExprNodePtr falseBranch;
        explicit ConditionNode(ExprNodePtr condition, ExprNodePtr trueBranch, ExprNodePtr falseBranch) :
            condition(std::move(condition)), trueBranch(std::move(trueBranch)), falseBranch(std::move(falseBranch)) {}
        ASTNodeGen traverse() {
            co_yield condition;
            co_yield trueBranch;
            co_yield falseBranch;
        }
        AST::TypeNodePtr evalType() { return nullptr; }
    };

#define EXPR_NODE_ASSERT(Name, ...)                                                                                    \
    static_assert(ExprNodePtrConcept<Name##Ptr>, "ExprNodePtrConcept<" #Name "Ptr> is not satisfied.");

    TINY_COBALT_AST_EXPR_NODES(EXPR_NODE_ASSERT);

#undef EXPR_NODE_ASSERT

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_