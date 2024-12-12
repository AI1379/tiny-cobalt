//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_TYPENODEIMPL_H_
#define TINY_COBALT_SRC_AST_TYPENODEIMPL_H_

#include "AST/ExprNode.h"
#include "AST/TypeNode.h"
#include "Common/Utility.h"

#include <vector>

namespace TinyCobalt::AST {
    struct SimpleTypeNode {
        const std::string name;
        explicit SimpleTypeNode(std::string name) : name(std::move(name)) {}
    };

    struct FuncTypeNode {
        const TypeNodePtr returnType;
        const std::vector<TypeNodePtr> paramTypes;
        FuncTypeNode(TypeNodePtr returnType, std::vector<TypeNodePtr> paramTypes) :
            returnType(std::move(returnType)), paramTypes(std::move(paramTypes)) {}
    };

    struct ComplexTypeNode {
        const std::string templateName;
        const std::vector<Utility::MergedVariant<ConstExprPtr, TypeNodePtr>> templateArgs;
        explicit ComplexTypeNode(std::string templateName,
                                 std::vector<Utility::MergedVariant<ConstExprPtr, TypeNodePtr>> templateArgs) :
            templateName(std::move(templateName)), templateArgs(std::move(templateArgs)) {}
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