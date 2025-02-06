//
// Created by Renatus Madrigal on 01/19/2025
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_
#define TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_

#include <cstddef>
#include <magic_enum.hpp>
#include <variant>
#include "AST/ASTNode.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Assert.h"
#include "Common/Utility.h"

namespace TinyCobalt::AST {

    // TypeNode

    // TODO: implement convertibleTo.
    struct SimpleTypeNode : public EnableThisPointer<SimpleTypeNode> {
        const std::string name;
        using TypeDefPtr = std::variant<AST::AliasDefPtr, AST::StructDefPtr, AST::SimpleTypePtr, std::nullptr_t>;
        TypeDefPtr def = nullptr;
        explicit SimpleTypeNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse();
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const;
    };

    struct FuncTypeNode : public EnableThisPointer<FuncTypeNode> {
        const TypeNodePtr returnType;
        const std::vector<TypeNodePtr> paramTypes;
        FuncTypeNode(TypeNodePtr returnType, std::vector<TypeNodePtr> paramTypes) :
            returnType(std::move(returnType)), paramTypes(std::move(paramTypes)) {}
        FuncTypeNode(TypeNodePtr returnType) : returnType(std::move(returnType)), paramTypes() {}
        ASTNodeGen traverse();
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const;
    };
    /**
     * We do not use C-style pointer and array. Instead, we decide to use a C++ template-like form to represent complex
     * types. For example, `int *` is represented as Pointer<int> in grammar and in AST the node is
     * ComplexTypeNode("Pointer", {SimpleTypeNode("int")}). The grammar is more readable and the AST is more flexible,
     * because complex function pointer and pointer array can be more structured.
     */
    struct ComplexTypeNode : public EnableThisPointer<ComplexTypeNode> {
        using TemplateArgType = std::variant<TypeNodePtr, ConstExprPtr>;
        struct Visitor;
        const std::string templateName;
        const std::vector<TemplateArgType> templateArgs;
        explicit ComplexTypeNode(std::string templateName, std::vector<TemplateArgType> templateArgs) :
            templateName(std::move(templateName)), templateArgs(std::move(templateArgs)) {}

        ASTNodeGen traverse();
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
        Common::JSON toJSON() const;
    };

    namespace BuiltInType {
        const SimpleTypeNode Int("int");
        const SimpleTypeNode UInt("uint");
        const SimpleTypeNode Float("float");
        const SimpleTypeNode Bool("bool");
        const SimpleTypeNode Char("char");
        const SimpleTypeNode Void("void");
        inline constexpr SimpleTypePtr findType(const std::string &name) {
            static auto kIntPtr = std::make_shared<SimpleTypeNode>(Int);
            static auto kUIntPtr = std::make_shared<SimpleTypeNode>(UInt);
            static auto kFloatPtr = std::make_shared<SimpleTypeNode>(Float);
            static auto kBoolPtr = std::make_shared<SimpleTypeNode>(Bool);
            static auto kCharPtr = std::make_shared<SimpleTypeNode>(Char);
            static auto kVoidPtr = std::make_shared<SimpleTypeNode>(Void);
            if (name == "int") {
                return kIntPtr;
            } else if (name == "uint") {
                return kUIntPtr;
            } else if (name == "float") {
                return kFloatPtr;
            } else if (name == "bool") {
                return kBoolPtr;
            } else if (name == "char") {
                return kCharPtr;
            } else if (name == "void") {
                return kVoidPtr;
            }
            return nullptr;
        }
    } // namespace BuiltInType

    // ExprNode
    // TODO: Compile-time evaluation
    // FIXME: Implement exprType()
    struct ConstExprNode : public EnableThisPointer<ConstExprNode> {
        const std::string value;
        const ConstExprType type;
        explicit ConstExprNode(std::string value, ConstExprType type) : value(value), type(type) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    struct VariableNode : public EnableThisPointer<VariableNode> {
        const std::string name;
        VariableDefPtr def = nullptr;
        explicit VariableNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    // TODO: support infix for binary function like Haskell
    struct BinaryNode : public EnableThisPointer<BinaryNode> {
        BinaryOp op;
        ExprNodePtr lhs;
        ExprNodePtr rhs;
        explicit BinaryNode(ExprNodePtr lhs, BinaryOp op, ExprNodePtr rhs) :
            op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    struct UnaryNode : public EnableThisPointer<UnaryNode> {
        const UnaryOp op;
        ExprNodePtr operand;
        explicit UnaryNode(UnaryOp op, ExprNodePtr operand) : op(std::move(op)), operand(std::move(operand)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    // Operators with multiple params, for example, operator[].
    // Because we allow operator overload, thus this support is necessary.
    struct MultiaryNode : public EnableThisPointer<MultiaryNode> {
        const MultiaryOp op;
        ExprNodePtr object;
        std::vector<ExprNodePtr> operands;
        explicit MultiaryNode(MultiaryOp op, ExprNodePtr obj, std::vector<ExprNodePtr> operands = {}) :
            op(op), object(obj), operands(std::move(operands)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    struct CastNode : public EnableThisPointer<CastNode> {
        const CastType op;
        TypeNodePtr type;
        ExprNodePtr operand;
        explicit CastNode(CastType op, TypeNodePtr type, ExprNodePtr operand) :
            op(op), type(std::move(type)), operand(std::move(operand)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    // Three way conditional operator
    struct ConditionNode : public EnableThisPointer<ConditionNode> {
        ExprNodePtr condition;
        ExprNodePtr trueBranch;
        ExprNodePtr falseBranch;
        explicit ConditionNode(ExprNodePtr condition, ExprNodePtr trueBranch, ExprNodePtr falseBranch) :
            condition(std::move(condition)), trueBranch(std::move(trueBranch)), falseBranch(std::move(falseBranch)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    struct MemberNode : public EnableThisPointer<MemberNode> {
        ExprNodePtr object;
        BinaryOp op;
        std::string member;
        explicit MemberNode(ExprNodePtr object, BinaryOp op, std::string member) :
            object(std::move(object)), op(op), member(std::move(member)) {}
        ASTNodeGen traverse();
        TypeNodePtr &exprType() {
            static TypeNodePtr ptr_ = nullptr;
            return ptr_;
        }
        Common::JSON toJSON() const;
    };

    namespace BuiltInOperator {
        // TODO: Implement builtin operators
    }

    // StmtNode
    // TODO: remove stmtFlag()

    struct IfNode : public EnableThisPointer<IfNode> {
        const ExprNodePtr condition;
        const StmtNodePtr thenStmt;
        const StmtNodePtr elseStmt;
        IfNode(ExprNodePtr condition, StmtNodePtr thenStmt, StmtNodePtr elseStmt) :
            condition(std::move(condition)), thenStmt(std::move(thenStmt)), elseStmt(std::move(elseStmt)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct WhileNode : public EnableThisPointer<WhileNode> {
        const ExprNodePtr condition;
        const StmtNodePtr body;
        WhileNode(ExprNodePtr condition, StmtNodePtr body) : condition(std::move(condition)), body(std::move(body)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct ForNode : public EnableThisPointer<ForNode> {
        const ExprNodePtr init;
        const ExprNodePtr condition;
        const ExprNodePtr step;
        const StmtNodePtr body;
        ForNode(ExprNodePtr init, ExprNodePtr condition, ExprNodePtr step, StmtNodePtr body) :
            init(std::move(init)), condition(std::move(condition)), step(std::move(step)), body(std::move(body)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct ReturnNode : public EnableThisPointer<ReturnNode> {
        const ExprNodePtr value;
        explicit ReturnNode(ExprNodePtr value) : value(std::move(value)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct BlockNode : public EnableThisPointer<BlockNode> {
        std::vector<StmtNodePtr> stmts;
        explicit BlockNode(std::vector<StmtNodePtr> stmts) : stmts(std::move(stmts)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct BreakNode : public EnableThisPointer<BreakNode> {
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct ContinueNode : public EnableThisPointer<ContinueNode> {
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct VariableDefNode : public EnableThisPointer<VariableDefNode> {
        const TypeNodePtr type;
        const std::string name;
        const ExprNodePtr init;
        VariableDefNode(TypeNodePtr type, std::string name, ExprNodePtr init = nullptr) :
            type(type), name(std::move(name)), init(init) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
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
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
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
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct AliasDefNode : public EnableThisPointer<AliasDefNode> {
        const std::string name;
        const TypeNodePtr type;
        AliasDefNode(std::string name, TypeNodePtr type) : name(std::move(name)), type(std::move(type)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct ExprStmtNode : public EnableThisPointer<ExprStmtNode> {
        const ExprNodePtr expr;
        explicit ExprStmtNode(ExprNodePtr expr) : expr(std::move(expr)) {}
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

    struct EmptyStmtNode : public EnableThisPointer<EmptyStmtNode> {
        ASTNodeGen traverse();
        void stmtFlag() {}
        Common::JSON toJSON() const;
    };

#define TYPE_NODE_ASSERT(Name, ...) TINY_COBALT_CONCEPT_ASSERT(TypeNodePtrConcept, Name##Ptr);

    TINY_COBALT_AST_TYPE_NODES(TYPE_NODE_ASSERT);

#undef TYPE_NODE_ASSERT

#define EXPR_NODE_ASSERT(Name, ...) TINY_COBALT_CONCEPT_ASSERT(ExprNodePtrConcept, Name##Ptr);

    TINY_COBALT_AST_EXPR_NODES(EXPR_NODE_ASSERT);

#undef EXPR_NODE_ASSERT

#define STMT_NODE_ASSERT(Name, ...) TINY_COBALT_CONCEPT_ASSERT(StmtNodePtrConcept, Name##Ptr);

    TINY_COBALT_AST_STMT_NODES(STMT_NODE_ASSERT);

#undef STMT_NODE_ASSERT

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTNODEDECL_H_