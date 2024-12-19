//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_STMTNODEIMPL_H_
#define TINY_COBALT_SRC_AST_STMTNODEIMPL_H_

#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"

#include <vector>

namespace TinyCobalt::AST {

    struct IfNode {
        const ExprNodePtr condition;
        const StmtNodePtr thenStmt;
        const StmtNodePtr elseStmt;
        IfNode(ExprNodePtr condition, StmtNodePtr thenStmt, StmtNodePtr elseStmt) :
            condition(std::move(condition)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(condition);
            co_yield pro::make_proxy<TraverseableProxy>(thenStmt);
            // TODO: check if nullptr check is necessary
            co_yield pro::make_proxy<TraverseableProxy>(elseStmt);
        }
    };

    struct WhileNode {
        const ExprNodePtr condition;
        const StmtNodePtr body;
        WhileNode(ExprNodePtr condition, StmtNodePtr body) : condition(std::move(condition)), body(std::move(body)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(condition);
            co_yield pro::make_proxy<TraverseableProxy>(body);
        }
    };

    struct ForNode {
        const StmtNodePtr init;
        const ExprNodePtr condition;
        const ExprNodePtr step;
        const StmtNodePtr body;
        ForNode(StmtNodePtr init, ExprNodePtr condition, ExprNodePtr step, StmtNodePtr body) :
            init(std::move(init)), condition(std::move(condition)), step(std::move(step)), body(std::move(body)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(init);
            co_yield pro::make_proxy<TraverseableProxy>(condition);
            co_yield pro::make_proxy<TraverseableProxy>(step);
            co_yield pro::make_proxy<TraverseableProxy>(body);
        }
    };

    struct ReturnNode {
        const ExprNodePtr value;
        explicit ReturnNode(ExprNodePtr value) : value(std::move(value)) {}
        TraverseableGen traverse() { co_yield pro::make_proxy<TraverseableProxy>(value); }
    };

    struct BlockNode {
        const std::vector<StmtNodePtr> stmts;
        explicit BlockNode(std::vector<StmtNodePtr> stmts) : stmts(std::move(stmts)) {}
        TraverseableGen traverse() {
            for (auto &stmt: stmts)
                co_yield pro::make_proxy<TraverseableProxy>(stmt);
        }
    };

    struct BreakNode {
        TraverseableGen traverse() { co_yield nullptr; }
    };

    struct ContinueNode {
        TraverseableGen traverse() { co_yield nullptr; }
    };

    struct VariableDefNode {
        const TypeNodePtr type;
        const std::string name;
        const ExprNodePtr init;
        VariableDefNode(TypeNodePtr type, std::string name, ExprNodePtr init) :
            type(std::move(type)), name(std::move(name)), init(std::move(init)) {}
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(type);
            co_yield pro::make_proxy<TraverseableProxy>(init);
        }
    };

    struct FuncDefNode {
        using ParamsElem = std::pair<TypeNodePtr, std::string>;
        const TypeNodePtr returnType;
        const std::string name;
        const std::vector<ParamsElem> params;
        const StmtNodePtr body;
        FuncDefNode(TypeNodePtr returnType, std::string name, std::vector<ParamsElem> params, StmtNodePtr body) :
            returnType(std::move(returnType)), name(std::move(name)), params(std::move(params)), body(std::move(body)) {
        }
        TraverseableGen traverse() {
            co_yield pro::make_proxy<TraverseableProxy>(returnType);
            for (auto &[type, _]: params)
                co_yield pro::make_proxy<TraverseableProxy>(type);
            co_yield pro::make_proxy<TraverseableProxy>(body);
        }
    };

    struct StructDefNode {
        using FieldsElem = std::pair<TypeNodePtr, std::string>;
        const std::string name;
        const std::vector<FieldsElem> fields;
        StructDefNode(std::string name, std::vector<FieldsElem> fields) :
            name(std::move(name)), fields(std::move(fields)) {}
        TraverseableGen traverse() {
            for (auto &[type, _]: fields)
                co_yield pro::make_proxy<TraverseableProxy>(type);
        }
    };

    struct AliasDefNode {
        const std::string name;
        const TypeNodePtr type;
        AliasDefNode(std::string name, TypeNodePtr type) : name(std::move(name)), type(std::move(type)) {}
        TraverseableGen traverse() { co_yield pro::make_proxy<TraverseableProxy>(type); }
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_STMTNODEIMPL_H_