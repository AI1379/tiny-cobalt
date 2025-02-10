//
// Created by Renatus Madrigal on 02/05/2025
//

#include "Semantic/TypeAnalyzer.h"
#include <memory>
#include "AST/ASTNodeDecl.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Assert.h"
#include "Common/Utility.h"

namespace TinyCobalt::Semantic {
    static auto kTemplateArgMatcher = Matcher{[](AST::TypeNodePtr node) { return node; },
                                              [](AST::ConstExprPtr node) { return AST::TypeNodePtr{nullptr}; }};

    AST::TypeNodePtr pointerTo(AST::TypeNodePtr ptr) {
        if (proxy_typeid(ptr).hash_code() != typeid(AST::ComplexTypePtr).hash_code())
            return nullptr;
        auto cplx = proxy_cast<AST::ComplexTypePtr>(ptr);
        if (cplx->templateName != "Pointer")
            return nullptr;
        return std::visit(kTemplateArgMatcher, cplx->templateArgs.front());
    }

    AST::VisitorState TypeAnalyzer::afterSubtreeImpl(AST::ASTNodePtr node) {
        auto matcher = Matcher{
#define REG_ANALYZER(Name, ...) [&](AST::Name##Ptr node) { return analyzeType(node); },
                TINY_COBALT_AST_EXPR_NODES(REG_ANALYZER)
#undef REG_ANALYZER
        };
        return visit(matcher, node);
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::ConstExprPtr ptr) {
        switch (ptr->type) {
            case AST::ConstExprType::Int:
            case AST::ConstExprType::HexInt:
            case AST::ConstExprType::OctInt:
            case AST::ConstExprType::BinInt: {
                ptr->exprType() = AST::BuiltInType::findType("int");
                break;
            }
            case AST::ConstExprType::Float: {
                ptr->exprType() = AST::BuiltInType::findType("float");
                break;
            }
            case AST::ConstExprType::Bool: {
                ptr->exprType() = AST::BuiltInType::findType("bool");
                break;
            }
            case AST::ConstExprType::String: {
                ptr->exprType() = std::make_shared<AST::ComplexTypeNode>(
                        "Pointer",
                        std::vector{AST::ComplexTypeNode::TemplateArgType(AST::BuiltInType::findType("char"))});
                break;
            }
            case AST::ConstExprType::Char: {
                ptr->exprType() = AST::BuiltInType::findType("char");
                break;
            }
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::VariablePtr ptr) {
        if (ptr->def == nullptr) {
            throw std::runtime_error("Variable " + ptr->name + " is not defined");
        }
        ptr->exprType() = ptr->def->type;
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::BinaryPtr ptr) {
        switch (ptr->op) {
            case AST::BinaryOp::Add:
            case AST::BinaryOp::Sub:
            case AST::BinaryOp::Mul:
            case AST::BinaryOp::Div:
            case AST::BinaryOp::Mod:
            case AST::BinaryOp::BitAnd:
            case AST::BinaryOp::BitOr:
            case AST::BinaryOp::BitXor:
            case AST::BinaryOp::BitLShift:
            case AST::BinaryOp::BitRShift: {
                // TODO: unsigned int support
                ptr->exprType() = AST::BuiltInType::findType("int");
                break;
            }
            case AST::BinaryOp::And:
            case AST::BinaryOp::Or:
            case AST::BinaryOp::Eq:
            case AST::BinaryOp::Ne:
            case AST::BinaryOp::Less:
            case AST::BinaryOp::Greater:
            case AST::BinaryOp::Leq:
            case AST::BinaryOp::Geq: {
                ptr->exprType() = AST::BuiltInType::findType("bool");
                break;
            }
            case AST::BinaryOp::Assign: {
                ptr->exprType() = ptr->rhs->exprType();
                break;
            }
            // TODO: Split compound assignment to assignment and operator for operator overloading
            case AST::BinaryOp::AddAssign:
            case AST::BinaryOp::SubAssign:
            case AST::BinaryOp::MulAssign:
            case AST::BinaryOp::DivAssign:
            case AST::BinaryOp::ModAssign:
            case AST::BinaryOp::BitAndAssign:
            case AST::BinaryOp::BitOrAssign:
            case AST::BinaryOp::BitXorAssign:
            case AST::BinaryOp::BitLShiftAssign:
            case AST::BinaryOp::BitRShiftAssign: {
                TINY_COBALT_ASSERT(ptr->exprType()->convertibleTo(AST::BuiltInType::findType("int")),
                                   "No valid operator for the operand.");
                ptr->exprType() = AST::BuiltInType::findType("int");
            }
            // TODO: remove this case
            case AST::BinaryOp::Member:
            case AST::BinaryOp::PtrMember:
                break;
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::UnaryPtr ptr) {
        switch (ptr->op) {
            case AST::UnaryOp::Positive:
            case AST::UnaryOp::Negative:
            case AST::UnaryOp::BitNot:
            case AST::UnaryOp::PreInc:
            case AST::UnaryOp::PreDec:
            case AST::UnaryOp::PostInc:
            case AST::UnaryOp::PostDec: {
                ptr->exprType() = AST::BuiltInType::findType("int");
                break;
            }
            case AST::UnaryOp::Not: {
                ptr->exprType() = AST::BuiltInType::findType("bool");
                break;
            }
            case AST::UnaryOp::Addr: {
                ptr->exprType() = pointerTo(ptr->operand->exprType());
                break;
            }
            case AST::UnaryOp::Deref: {
                auto t = ptr->operand->exprType();
                ptr->exprType() = std::make_shared<AST::ComplexTypeNode>(
                        "Pointer", std::vector{AST::ComplexTypeNode::TemplateArgType(t)});
                break;
            }
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::MultiaryPtr ptr) {
        switch (ptr->op) {
            case AST::MultiaryOp::Subscript: {
                // TODO: Implement type analyzer for subscript overload
                auto def = ptr->object->exprType();
                TINY_COBALT_ASSERT(ptr->operands.size() == 1 && pointerType<AST::ConstExprPtr>(ptr->operands.front()),
                                   "Invalid subscript");
                TINY_COBALT_ASSERT(pointerType<AST::ComplexTypePtr>(def), "Not a pointer or array");
                auto cplx = proxy_cast<AST::ComplexTypePtr>(def);
                TINY_COBALT_ASSERT(cplx->templateName == "Pointer" || cplx->templateName == "Array",
                                   "Not a pointer or array");
                ptr->exprType() = std::visit(kTemplateArgMatcher, cplx->templateArgs.front());
                TINY_COBALT_ASSERT(ptr->exprType(), "Invalid subscript");
                break;
            }
            case AST::MultiaryOp::FuncCall: {
                // TODO: Overloaded function
                auto def = ptr->object->exprType();
                TINY_COBALT_ASSERT(pointerType<AST::FuncDefPtr>(def), "Not a function");
                auto func_def = proxy_cast<AST::FuncDefPtr>(def);
                ptr->exprType() = func_def->returnType;
                break;
            }
            case AST::MultiaryOp::Comma:
                break;
        }
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::CastPtr ptr) {
        ptr->exprType() = ptr->type;
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::ConditionPtr ptr) {
        TINY_COBALT_ASSERT(ptr->trueBranch->exprType()->convertibleTo(ptr->falseBranch->exprType()), "Type mismatch");
        ptr->exprType() = ptr->trueBranch->exprType();
        return AST::VisitorState::Normal;
    }

    AST::VisitorState TypeAnalyzer::analyzeType(AST::MemberPtr ptr) {
        auto def = ptr->object->exprType();
        TINY_COBALT_ASSERT(proxy_typeid(def).hash_code() == typeid(AST::StructDefPtr).hash_code(), "Not a struct");
        auto struct_def = proxy_cast<AST::StructDefPtr>(def);
        for (auto &x: struct_def->fields) {
            if (x->name == ptr->member) {
                ptr->exprType() = x->type;
                return AST::VisitorState::Normal;
            }
        }
        return AST::VisitorState::Exit;
    }

} // namespace TinyCobalt::Semantic