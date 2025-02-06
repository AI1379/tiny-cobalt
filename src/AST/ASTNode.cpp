//
// Created by Renatus Madrigal on 02/05/2025
//

#include "AST/ASTNode.h"
#include "AST/ASTNodeDecl.h"
#include "AST/TypeNode.h"
#include "Common/JSON.h"

namespace TinyCobalt::AST {

    // TypeNode
    ASTNodeGen SimpleTypeNode::traverse() { co_return; }

    Common::JSON SimpleTypeNode::toJSON() const {
        Common::JSON json;
        json["type"] = "SimpleType";
        json["name"] = name;
        return json;
    }

    ASTNodeGen FuncTypeNode::traverse() {
        co_yield returnType;
        for (auto &paramType: paramTypes) {
            co_yield paramType;
        }
    }

    Common::JSON FuncTypeNode::toJSON() const {
        Common::JSON json;
        json["type"] = "FuncType";
        json["return_type"] = returnType->toJSON();
        json["param_types"] = Common::JSON::array();
        for (const auto &paramType: paramTypes) {
            json["param_types"].push_back(paramType->toJSON());
        }
        return json;
    }

    struct ComplexTypeNode::Visitor {
        ASTNodePtr operator()(const TypeNodePtr &type) const { return type; }
        ASTNodePtr operator()(const ConstExprPtr &expr) const { return expr; }
    };

    ASTNodeGen ComplexTypeNode::traverse() {
        for (auto &arg: templateArgs) {
            co_yield std::visit(Visitor{}, arg);
        }
    }

    Common::JSON ComplexTypeNode::toJSON() const {
        Common::JSON json;
        json["type"] = "ComplexType";
        json["template_name"] = templateName;
        json["template_args"] = Common::JSON::array();
        for (const auto &arg: templateArgs) {
            json["template_args"].push_back(std::visit(Visitor{}, arg)->toJSON());
        }
        return json;
    }

    // ExprNode

    ASTNodeGen ConstExprNode::traverse() { co_return; }

    Common::JSON ConstExprNode::toJSON() const {
        Common::JSON json;
        json["type"] = "ConstExpr";
        json["value"] = value;
        json["expr_type"] = magic_enum::enum_name(type);
        return json;
    }

    ASTNodeGen VariableNode::traverse() { co_return; }

    Common::JSON VariableNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Variable";
        json["name"] = name;
        return json;
    }

    ASTNodeGen BinaryNode::traverse() {
        co_yield lhs;
        co_yield rhs;
    }

    Common::JSON BinaryNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Binary";
        json["op"] = magic_enum::enum_name(op);
        json["lhs"] = lhs->toJSON();
        json["rhs"] = rhs->toJSON();
        return json;
    }

    ASTNodeGen UnaryNode::traverse() { co_yield operand; }

    Common::JSON UnaryNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Unary";
        json["op"] = magic_enum::enum_name(op);
        json["operand"] = operand->toJSON();
        return json;
    }

    ASTNodeGen MultiaryNode::traverse() {
        co_yield object;
        for (const auto &operand: operands) {
            co_yield operand;
        }
    }

    Common::JSON MultiaryNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Multiary";
        json["op"] = magic_enum::enum_name(op);
        json["object"] = object->toJSON();
        json["operands"] = Common::JSON::array();
        for (const auto &operand: operands) {
            json["operands"].push_back(operand->toJSON());
        }
        return json;
    }

    ASTNodeGen CastNode::traverse() {
        co_yield type;
        co_yield operand;
    }

    Common::JSON CastNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Cast";
        json["op"] = magic_enum::enum_name(op);
        json["cast_type"] = type->toJSON();
        json["operand"] = operand->toJSON();
        return json;
    }

    ASTNodeGen ConditionNode::traverse() {
        co_yield condition;
        co_yield trueBranch;
        co_yield falseBranch;
    }

    Common::JSON ConditionNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Condition";
        json["condition"] = condition->toJSON();
        json["true_branch"] = trueBranch->toJSON();
        json["false_branch"] = falseBranch->toJSON();
        return json;
    }

    ASTNodeGen MemberNode::traverse() { co_yield object; }

    Common::JSON MemberNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Member";
        json["object"] = object->toJSON();
        json["op"] = magic_enum::enum_name(op);
        json["member"] = member;
        return json;
    }

    // StmtNode

    ASTNodeGen IfNode::traverse() {
        co_yield condition;
        co_yield thenStmt;
        co_yield elseStmt;
    }

    Common::JSON IfNode::toJSON() const {
        Common::JSON json;
        json["type"] = "If";
        json["condition"] = condition->toJSON();
        json["then_stmt"] = thenStmt->toJSON();
        json["else_stmt"] = elseStmt ? elseStmt->toJSON() : nullptr;
        return json;
    }

    ASTNodeGen WhileNode::traverse() {
        co_yield condition;
        co_yield body;
    }

    Common::JSON WhileNode::toJSON() const {
        Common::JSON json;
        json["type"] = "While";
        json["condition"] = condition->toJSON();
        json["body"] = body->toJSON();
        return json;
    }

    ASTNodeGen ForNode::traverse() {
        co_yield init;
        co_yield condition;
        co_yield step;
        co_yield body;
    }

    Common::JSON ForNode::toJSON() const {
        Common::JSON json;
        json["type"] = "For";
        json["init"] = init->toJSON();
        json["condition"] = condition->toJSON();
        json["step"] = step->toJSON();
        json["body"] = body->toJSON();
        return json;
    }

    ASTNodeGen ReturnNode::traverse() { co_yield value; }

    Common::JSON ReturnNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Return";
        json["value"] = value ? value->toJSON() : nullptr;
        return json;
    }

    ASTNodeGen BlockNode::traverse() {
        for (auto &stmt: stmts) {
            co_yield stmt;
        }
    }

    Common::JSON BlockNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Block";
        json["stmts"] = Common::JSON::array();
        for (const auto &stmt: stmts) {
            json["stmts"].push_back(stmt->toJSON());
        }
        return json;
    }

    ASTNodeGen BreakNode::traverse() { co_return; }

    Common::JSON BreakNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Break";
        return json;
    }

    ASTNodeGen ContinueNode::traverse() { co_return; }

    Common::JSON ContinueNode::toJSON() const {
        Common::JSON json;
        json["type"] = "Continue";
        return json;
    }

    ASTNodeGen VariableDefNode::traverse() {
        co_yield type;
        co_yield init;
    }

    Common::JSON VariableDefNode::toJSON() const {
        Common::JSON json;
        json["type"] = "VariableDef";
        json["type"] = type->toJSON();
        json["name"] = name;
        json["init"] = init ? init->toJSON() : nullptr;
        return json;
    }

    ASTNodeGen FuncDefNode::traverse() {
        co_yield returnType;
        for (auto &param: params) {
            co_yield param;
        }
        co_yield body;
    }

    Common::JSON FuncDefNode::toJSON() const {
        Common::JSON json;
        json["type"] = "FuncDef";
        json["return_type"] = returnType->toJSON();
        json["name"] = name;
        json["params"] = Common::JSON::array();
        for (const auto &param: params) {
            json["params"].push_back(param->toJSON());
        }
        json["body"] = body->toJSON();
        return json;
    }

    ASTNodeGen StructDefNode::traverse() {
        for (auto &field: fields) {
            co_yield field;
        }
    }

    Common::JSON StructDefNode::toJSON() const {
        Common::JSON json;
        json["type"] = "StructDef";
        json["name"] = name;
        json["fields"] = Common::JSON::array();
        for (const auto &field: fields) {
            json["fields"].push_back(field->toJSON());
        }
        return json;
    }

    ASTNodeGen AliasDefNode::traverse() { co_yield type; }

    Common::JSON AliasDefNode::toJSON() const {
        Common::JSON json;
        json["type"] = "AliasDef";
        json["name"] = name;
        json["type"] = type->toJSON();
        return json;
    }

    ASTNodeGen ExprStmtNode::traverse() { co_yield expr; }

    Common::JSON ExprStmtNode::toJSON() const {
        Common::JSON json;
        json["type"] = "ExprStmt";
        json["expr"] = expr->toJSON();
        return json;
    }

    ASTNodeGen EmptyStmtNode::traverse() { co_return; }

    Common::JSON EmptyStmtNode::toJSON() const {
        Common::JSON json;
        json["type"] = "EmptyStmt";
        return json;
    }

} // namespace TinyCobalt::AST