//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_
#define TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/ExprNode.h"
#include "AST/TypeNode.h"

#include <proxy.h>
#include <string>
#include <vector>

namespace TinyCobalt::AST {

    struct ConstExprNode {
        const std::string value;
        TypeNodePtr type;
        explicit ConstExprNode(std::string value) : value(value), type(nullptr) {}
        TraverseableGen traverse() { co_yield pro::make_proxy<TraverseableProxy>(type); }
    };

    struct AssignNode {
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit AssignNode(ExprNodePtr lhs, ExprNodePtr rhs) : lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(lhs);
            co_yield pro::make_proxy<TraverseableProxy>(rhs);
        }
    };

    struct BinaryNode {
        const BinaryOp op;
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit BinaryNode(BinaryOp op, ExprNodePtr lhs, ExprNodePtr rhs) :
            op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(lhs);
            co_yield pro::make_proxy<TraverseableProxy>(rhs);
        }
    };

    struct UnaryNode {
        const UnaryOp op;
        ExprNodePtr operand;
        explicit UnaryNode(UnaryOp op, ExprNodePtr operand) : op(std::move(op)), operand(std::move(operand)) {}
        TraverseableGen traverse() { co_yield pro::make_proxy<TraverseableProxy>(operand); }
    };

    struct CastNode {
        TypeNodePtr type;
        ExprNodePtr operand;
        explicit CastNode(TypeNodePtr type, ExprNodePtr operand) : type(std::move(type)), operand(std::move(operand)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(type);
            co_yield pro::make_proxy<TraverseableProxy>(operand);
        }
    };

    struct ConditionNode {
        ExprNodePtr condition;
        ExprNodePtr trueBranch;
        ExprNodePtr falseBranch;
        explicit ConditionNode(ExprNodePtr condition, ExprNodePtr trueBranch, ExprNodePtr falseBranch) :
            condition(std::move(condition)), trueBranch(std::move(trueBranch)), falseBranch(std::move(falseBranch)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(condition);
            co_yield pro::make_proxy<TraverseableProxy>(trueBranch);
            co_yield pro::make_proxy<TraverseableProxy>(falseBranch);
        }
    };

    struct FuncCallNode {
        ExprNodePtr func;
        std::vector<ExprNodePtr> args;
        explicit FuncCallNode(ExprNodePtr func, std::vector<ExprNodePtr> args) :
            func(std::move(func)), args(std::move(args)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(func);
            for (auto &arg: args)
                co_yield pro::make_proxy<TraverseableProxy>(arg);
        }
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_EXPRNODEIMPL_H_