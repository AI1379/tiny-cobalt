//
// Created by Renatus Madrigal on 01/19/2025
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_
#define TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_

#include <magic_enum.hpp>
#include "AST/ASTNode.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"

namespace TinyCobalt::AST {

    // TypeNode

    // TODO: implement convertibleTo.
    struct SimpleTypeNode : public EnableThisPointer<SimpleTypeNode> {
        const std::string name;
        explicit SimpleTypeNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse() { co_yield nullptr; }
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "SimpleType";
            json["name"] = name;
            return json;
        }
    };

    struct FuncTypeNode : public EnableThisPointer<FuncTypeNode> {
        const TypeNodePtr returnType;
        const std::vector<TypeNodePtr> paramTypes;
        FuncTypeNode(TypeNodePtr returnType, std::vector<TypeNodePtr> paramTypes) :
            returnType(std::move(returnType)), paramTypes(std::move(paramTypes)) {}
        FuncTypeNode(TypeNodePtr returnType) : returnType(std::move(returnType)), paramTypes() {}
        ASTNodeGen traverse() {
            co_yield returnType;
            for (auto &paramType: paramTypes)
                co_yield paramType;
        }
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "FuncType";
            json["returnType"] = returnType->toJSON();
            json["paramTypes"] = Common::JSON::array();
            for (const auto &paramType: paramTypes) {
                json["paramTypes"].push_back(paramType->toJSON());
            }
            return json;
        }
    };
    /**
     * We do not use C-style pointer and array. Instead, we decide to use a C++ template-like form to represent complex
     * types. For example, `int *` is represented as Pointer<int> in grammar and in AST the node is
     * ComplexTypeNode("Pointer", {SimpleTypeNode("int")}). The grammar is more readable and the AST is more flexible,
     * because complex function pointer and pointer array can be more structured.
     */
    struct ComplexTypeNode : public EnableThisPointer<ComplexTypeNode> {
        const std::string templateName;
        const std::vector<TypeNodePtr> templateArgs;
        explicit ComplexTypeNode(std::string templateName, std::vector<TypeNodePtr> templateArgs) :
            templateName(std::move(templateName)), templateArgs(std::move(templateArgs)) {}

        ASTNodeGen traverse() {
            for (auto &arg: templateArgs)
                co_yield arg;
        }
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "ComplexType";
            json["templateName"] = templateName;
            json["templateArgs"] = Common::JSON::array();
            for (const auto &arg: templateArgs) {
                json["templateArgs"].push_back(arg->toJSON());
            }
            return json;
        }
    };

    namespace BuiltInType {
        const SimpleTypeNode Int("int");
        const SimpleTypeNode UInt("uint");
        const SimpleTypeNode Float("float");
        const SimpleTypeNode Bool("bool");
        const SimpleTypeNode Char("char");
        const SimpleTypeNode Void("void");
    } // namespace BuiltInType

#define TYPE_NODE_ASSERT(Name, ...)                                                                                    \
    static_assert(TypeNodePtrConcept<Name##Ptr>, "TypeNodePtrConcept<" #Name "Ptr> is not satisfied.");

    TINY_COBALT_AST_TYPE_NODES(TYPE_NODE_ASSERT);

#undef TYPE_NODE_ASSERT

    // ExprNode
    // TODO: Compile-time evaluation
    // FIXME: Implement evalType()
    struct ConstExprNode : public EnableThisPointer<ConstExprNode> {
        const std::string value;
        const ConstExprType expr_type;
        explicit ConstExprNode(std::string value, ConstExprType expr_type) : value(value), expr_type(expr_type) {}
        ASTNodeGen traverse() { co_return; }
        AST::TypeNodePtr evalType() { return nullptr; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "ConstExpr";
            json["value"] = value;
            json["expr_type"] = magic_enum::enum_name(expr_type);
            return json;
        }
    };

    struct VariableNode : public EnableThisPointer<VariableNode> {
        const std::string name;
        explicit VariableNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse() { co_return; }
        AST::TypeNodePtr evalType() { return nullptr; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Variable";
            json["name"] = name;
            return json;
        }
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
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Binary";
            json["op"] = magic_enum::enum_name(op);
            json["lhs"] = lhs->toJSON();
            json["rhs"] = rhs->toJSON();
            return json;
        }
    };

    struct UnaryNode : public EnableThisPointer<UnaryNode> {
        const UnaryOp op;
        ExprNodePtr operand;
        explicit UnaryNode(UnaryOp op, ExprNodePtr operand) : op(std::move(op)), operand(std::move(operand)) {}
        ASTNodeGen traverse() { co_yield operand; }
        AST::TypeNodePtr evalType() { return nullptr; }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Unary";
            json["op"] = magic_enum::enum_name(op);
            json["operand"] = operand->toJSON();
            return json;
        }
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
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Multiary";
            json["op"] = magic_enum::enum_name(op);
            json["object"] = object;
            json["operands"] = Common::JSON::array();
            for (const auto &operand: operands) {
                json["operands"].push_back(operand->toJSON());
            }
            return json;
        }
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
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Cast";
            json["op"] = magic_enum::enum_name(op);
            json["type"] = type->toJSON();
            json["operand"] = operand->toJSON();
            return json;
        }
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
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "Condition";
            json["condition"] = condition->toJSON();
            json["trueBranch"] = trueBranch->toJSON();
            json["falseBranch"] = falseBranch->toJSON();
            return json;
        }
    };

#define EXPR_NODE_ASSERT(Name, ...)                                                                                    \
    static_assert(ExprNodePtrConcept<Name##Ptr>, "ExprNodePtrConcept<" #Name "Ptr> is not satisfied.");

    TINY_COBALT_AST_EXPR_NODES(EXPR_NODE_ASSERT);

#undef EXPR_NODE_ASSERT

    // StmtNode
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

#endif // TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_