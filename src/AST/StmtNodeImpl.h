//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_STMTNODEIMPL_H_
#define TINY_COBALT_SRC_AST_STMTNODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/StmtNode.h"
#include "Common/JSON.h"

#include <memory>
#include <vector>

namespace TinyCobalt::AST {
    // TODO: remove stmtFlag()

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
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "If";
            json["condition"] = condition->toJSON();
            json["thenStmt"] = thenStmt->toJSON();
            json["elseStmt"] = elseStmt ? elseStmt->toJSON() : nullptr;
            return json;
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
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "While";
            json["condition"] = condition->toJSON();
            json["body"] = body->toJSON();
            return json;
        }
    };

    struct ForNode : public EnableThisPointer<ForNode> {
        const ExprNodePtr init;
        const ExprNodePtr condition;
        const ExprNodePtr step;
        const StmtNodePtr body;
        ForNode(ExprNodePtr init, ExprNodePtr condition, ExprNodePtr step, StmtNodePtr body) :
            init(std::move(init)), condition(std::move(condition)), step(std::move(step)), body(std::move(body)) {}
        ASTNodeGen traverse() {
            co_yield init;
            co_yield condition;
            co_yield step;
            co_yield body;
        }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "For";
            json["init"] = init->toJSON();
            json["condition"] = condition->toJSON();
            json["step"] = step->toJSON();
            json["body"] = body->toJSON();
            return json;
        }
    };

    struct ReturnNode : public EnableThisPointer<ReturnNode> {
        const ExprNodePtr value;
        explicit ReturnNode(ExprNodePtr value) : value(std::move(value)) {}
        ASTNodeGen traverse() { co_yield value; }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Return";
            json["value"] = value ? value->toJSON() : nullptr;
            return json;
        }
    };

    struct BlockNode : public EnableThisPointer<BlockNode> {
        std::vector<StmtNodePtr> stmts;
        explicit BlockNode(std::vector<StmtNodePtr> stmts) : stmts(std::move(stmts)) {}
        ASTNodeGen traverse() {
            for (auto &stmt: stmts)
                co_yield stmt;
        }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Block";
            json["stmts"] = Common::JSON::array();
            for (const auto &stmt: stmts) {
                json["stmts"].push_back(stmt->toJSON());
            }
            return json;
        }
    };

    struct BreakNode : public EnableThisPointer<BreakNode> {
        ASTNodeGen traverse() { co_yield nullptr; }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Break";
            return json;
        }
    };

    struct ContinueNode : public EnableThisPointer<ContinueNode> {
        ASTNodeGen traverse() { co_yield nullptr; }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Continue";
            return json;
        }
    };

    struct VariableDefNode : public EnableThisPointer<VariableDefNode> {
        const TypeNodePtr type;
        const std::string name;
        const ExprNodePtr init;
        VariableDefNode(TypeNodePtr type, std::string name, ExprNodePtr init = nullptr) :
            type(type), name(std::move(name)), init(init) {}
        ASTNodeGen traverse() {
            co_yield type;
            co_yield init;
        }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "VariableDef";
            json["type"] = type->toJSON();
            json["name"] = name;
            json["init"] = init ? init->toJSON() : nullptr;
            return json;
        }
    };

    struct FuncDefNode : public EnableThisPointer<FuncDefNode> {
        // using ParamsElem = VariableDefPtr;
        struct ParamsElemNode : VariableDefNode {
            template<typename... Args>
            ParamsElemNode(Args &&...args) : VariableDefNode(std::forward<Args>(args)...) {}
        };
        using ParamsElem = std::shared_ptr<ParamsElemNode>;
        const TypeNodePtr returnType;
        const std::string name;
        const std::vector<ParamsElem> params;
        const StmtNodePtr body;
        FuncDefNode(TypeNodePtr returnType, std::string name, std::vector<ParamsElem> params, StmtNodePtr body) :
            returnType(std::move(returnType)), name(std::move(name)), params(std::move(params)), body(std::move(body)) {
        }
        FuncDefNode(TypeNodePtr returnType, std::string name, StmtNodePtr body) :
            returnType(std::move(returnType)), name(std::move(name)), params(), body(std::move(body)) {}
        ASTNodeGen traverse() {
            co_yield returnType;
            for (auto &param: params)
                co_yield param;
            co_yield body;
        }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "FuncDef";
            json["returnType"] = returnType->toJSON();
            json["name"] = name;
            json["params"] = Common::JSON::array();
            for (const auto &param: params) {
                json["params"].push_back(param->toJSON());
            }
            json["body"] = body->toJSON();
            return json;
        }
    };

    struct StructDefNode : public EnableThisPointer<StructDefNode> {
        // using FieldsElem = VariableDefPtr;
        struct FieldsElemNode : VariableDefNode {
            template<typename... Args>
            FieldsElemNode(Args &&...args) : VariableDefNode(std::forward<Args>(args)...) {}
        };
        using FieldsElem = std::shared_ptr<FieldsElemNode>;
        const std::string name;
        const std::vector<FieldsElem> fields;
        StructDefNode(std::string name, std::vector<FieldsElem> fields) :
            name(std::move(name)), fields(std::move(fields)) {}
        explicit StructDefNode(std::string name) : name(std::move(name)), fields() {}
        ASTNodeGen traverse() {
            for (auto field: fields)
                co_yield field;
        }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "StructDef";
            json["name"] = name;
            json["fields"] = Common::JSON::array();
            for (const auto &field: fields) {
                json["fields"].push_back(field->toJSON());
            }
            return json;
        }
    };

    struct AliasDefNode : public EnableThisPointer<AliasDefNode> {
        const std::string name;
        const TypeNodePtr type;
        AliasDefNode(std::string name, TypeNodePtr type) : name(std::move(name)), type(std::move(type)) {}
        ASTNodeGen traverse() { co_yield type; }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "AliasDef";
            json["name"] = name;
            json["type"] = type->toJSON();
            return json;
        }
    };

    struct ExprStmtNode : public EnableThisPointer<ExprStmtNode> {
        const ExprNodePtr expr;
        explicit ExprStmtNode(ExprNodePtr expr) : expr(std::move(expr)) {}
        ASTNodeGen traverse() { co_yield expr; }
        void stmtFlag() {}
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "ExprStmt";
            json["expr"] = expr->toJSON();
            return json;
        }
    };

#define STMT_NODE_ASSERT(Name, ...)                                                                                    \
    static_assert(StmtNodePtrConcept<Name##Ptr>, "StmtNodePtrConcept<" #Name "Ptr> is not satisfied.");

    TINY_COBALT_AST_STMT_NODES(STMT_NODE_ASSERT);

#undef STMT_NODE_ASSERT

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_STMTNODEIMPL_H_