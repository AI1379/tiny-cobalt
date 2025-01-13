//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_STMTNODEIMPL_H_
#define TINY_COBALT_SRC_AST_STMTNODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"

#include <vector>

namespace TinyCobalt::AST {

    struct IfNode : public EnableThisPointer<IfNode> {
        const ExprNodePtr condition;
        const StmtNodePtr thenStmt;
        const StmtNodePtr elseStmt;
        IfNode(ExprNodePtr condition, StmtNodePtr thenStmt, StmtNodePtr elseStmt) :
            condition(std::move(condition)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
        ASTNodeGen traverse() {
            co_yield condition;
            co_yield thenStmt;
            // TODO: check if nullptr check is necessary
            co_yield elseStmt;
        }
    };

    struct WhileNode : public EnableThisPointer<WhileNode> {
        const ExprNodePtr condition;
        const StmtNodePtr body;
        WhileNode(ExprNodePtr condition, StmtNodePtr body) : condition(std::move(condition)), body(std::move(body)) {}
        ASTNodeGen traverse() {
            co_yield condition;
            co_yield body;
        }
    };

    struct ForNode : public EnableThisPointer<ForNode> {
        const StmtNodePtr init;
        const ExprNodePtr condition;
        const ExprNodePtr step;
        const StmtNodePtr body;
        ForNode(StmtNodePtr init, ExprNodePtr condition, ExprNodePtr step, StmtNodePtr body) :
            init(std::move(init)), condition(std::move(condition)), step(std::move(step)), body(std::move(body)) {}
        ASTNodeGen traverse() {
            co_yield init;
            co_yield condition;
            co_yield step;
            co_yield body;
        }
    };

    struct ReturnNode : public EnableThisPointer<ReturnNode> {
        const ExprNodePtr value;
        explicit ReturnNode(ExprNodePtr value) : value(std::move(value)) {}
        ASTNodeGen traverse() { co_yield value; }
    };

    struct BlockNode : public EnableThisPointer<BlockNode> {
        std::vector<StmtNodePtr> stmts;
        explicit BlockNode(std::vector<StmtNodePtr> stmts) : stmts(std::move(stmts)) {}
        ASTNodeGen traverse() {
            for (auto &stmt: stmts)
                co_yield stmt;
        }
    };

    struct BreakNode : public EnableThisPointer<BreakNode> {
        ASTNodeGen traverse() { co_yield nullptr; }
    };

    struct ContinueNode : public EnableThisPointer<ContinueNode> {
        ASTNodeGen traverse() { co_yield nullptr; }
    };

    struct VariableDefNode : public EnableThisPointer<VariableDefNode> {
        const TypeNodePtr type;
        const std::string name;
        const ExprNodePtr init;
        VariableDefNode(TypeNodePtr type, std::string name, ExprNodePtr init) :
            type(std::move(type)), name(std::move(name)), init(std::move(init)) {}
        ASTNodeGen traverse() {
            co_yield type;
            co_yield init;
        }
    };

    struct FuncDefNode : public EnableThisPointer<FuncDefNode> {
        using ParamsElem = std::pair<TypeNodePtr, std::string>;
        const TypeNodePtr returnType;
        const std::string name;
        const std::vector<ParamsElem> params;
        const StmtNodePtr body;
        FuncDefNode(TypeNodePtr returnType, std::string name, std::vector<ParamsElem> params, StmtNodePtr body) :
            returnType(std::move(returnType)), name(std::move(name)), params(std::move(params)), body(std::move(body)) {
        }
        ASTNodeGen traverse() {
            co_yield returnType;
            for (auto &[type, _]: params)
                co_yield type;
            co_yield body;
        }
    };

    struct StructDefNode : public EnableThisPointer<StructDefNode> {
        using FieldsElem = std::pair<TypeNodePtr, std::string>;
        const std::string name;
        const std::vector<FieldsElem> fields;
        StructDefNode(std::string name, std::vector<FieldsElem> fields) :
            name(std::move(name)), fields(std::move(fields)) {}
        ASTNodeGen traverse() {
            for (auto &[type, _]: fields)
                co_yield type;
        }
    };

    struct AliasDefNode : public EnableThisPointer<AliasDefNode> {
        const std::string name;
        const TypeNodePtr type;
        AliasDefNode(std::string name, TypeNodePtr type) : name(std::move(name)), type(std::move(type)) {}
        ASTNodeGen traverse() { co_yield type; }
    };

    struct ExprStmtNode : public EnableThisPointer<ExprStmtNode> {
        const ExprNodePtr expr;
        explicit ExprStmtNode(ExprNodePtr expr) : expr(std::move(expr)) {}
        ASTNodeGen traverse() { co_yield expr; }
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_STMTNODEIMPL_H_