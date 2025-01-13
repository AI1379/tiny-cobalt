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
    // FIXME: Implement EvalType()

    struct ConstExprNode : EnableThisPointer<ConstExprNode> {
        const std::string value;
        TypeNodePtr type;
        explicit ConstExprNode(std::string value) : value(value), type(nullptr) {}
        ASTNodeGen traverse() { co_yield type; }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

    struct AssignNode : EnableThisPointer<AssignNode> {
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit AssignNode(ExprNodePtr lhs, ExprNodePtr rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        ASTNodeGen traverse() {
            co_yield lhs;
            co_yield rhs;
        }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

    struct BinaryNode : public EnableThisPointer<BinaryNode> {
        BinaryOp op;
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit BinaryNode(BinaryOp op, ExprNodePtr lhs, ExprNodePtr rhs) :
            op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        // BinaryNode(const BinaryNode &) = default;
        // BinaryNode &operator=(const BinaryNode &) = default;
        ASTNodeGen traverse() {
            co_yield lhs;
            co_yield rhs;
        }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

    struct UnaryNode : public EnableThisPointer<UnaryNode> {
        const UnaryOp op;
        ExprNodePtr operand;
        explicit UnaryNode(UnaryOp op, ExprNodePtr operand) : op(std::move(op)), operand(std::move(operand)) {}
        ASTNodeGen traverse() { co_yield operand; }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

    struct CastNode : public EnableThisPointer<CastNode> {
        TypeNodePtr type;
        ExprNodePtr operand;
        explicit CastNode(TypeNodePtr type, ExprNodePtr operand) : type(std::move(type)), operand(std::move(operand)) {}
        ASTNodeGen traverse() {
            co_yield type;
            co_yield operand;
        }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

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
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

    struct FuncCallNode : public EnableThisPointer<FuncCallNode> {
        ExprNodePtr func;
        std::vector<ExprNodePtr> args;
        explicit FuncCallNode(ExprNodePtr func, std::vector<ExprNodePtr> args = {}) :
            func(std::move(func)), args(std::move(args)) {}
        ASTNodeGen traverse() {
            co_yield func;
            for (auto &arg: args)
                co_yield arg;
        }
        AST::TypeNodePtr EvalType() { return nullptr; }
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_