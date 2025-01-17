//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_TYPENODEIMPL_H_
#define TINY_COBALT_SRC_AST_TYPENODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/TypeNode.h"
#include "Common/Utility.h"

#include <proxy.h>
#include <vector>

namespace TinyCobalt::AST {
    struct SimpleTypeNode : public EnableThisPointer<SimpleTypeNode> {
        const std::string name;
        explicit SimpleTypeNode(std::string name) : name(std::move(name)) {}
        ASTNodeGen traverse() { co_yield nullptr; }
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
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
    };
    /**
     * We do not use C-style pointer and array. Instead, we decide to use a C++ template-like form to represent complex
     * types. For example, `int *` is represented as Pointer<int> in grammar and in AST the node is
     * ComplexTypeNode("Pointer", {SimpleTypeNode("int")}). The grammar is more readable and the AST is more flexible,
     * because complex function pointer and pointer array can be more structured.
     */
    struct ComplexTypeNode : public EnableThisPointer<ComplexTypeNode> {
        const std::string templateName;
        // TODO: restrict the type of templateArgs
        const std::vector<ASTNodePtr> templateArgs;
        explicit ComplexTypeNode(std::string templateName, std::vector<ASTNodePtr> templateArgs) :
            templateName(std::move(templateName)), templateArgs(std::move(templateArgs)) {}

        ASTNodeGen traverse() {
            for (auto &arg: templateArgs)
                co_yield arg;
        }
        bool convertibleTo(const pro::proxy<TypeNodeProxy> &other) const { return false; }
    };

    namespace BuiltInType {
        const SimpleTypeNode Int("int");
        const SimpleTypeNode Float("float");
        const SimpleTypeNode Bool("bool");
        const SimpleTypeNode Char("char");
        const SimpleTypeNode Void("void");
    } // namespace BuiltInType

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_TYPENODEIMPL_H_