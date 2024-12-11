//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_TYPENODE_H_
#define TINY_COBALT_INCLUDE_AST_TYPENODE_H_

#include "AST/ExprNode.h"
#include "Common/Utility.h"

#include <memory>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#define TINY_COBALT_AST_TYPE_NODES(X, ...)                                                                             \
    X(SimpleType, __VA_ARGS__)                                                                                         \
    X(FuncType, __VA_ARGS__)                                                                                           \
    X(ComplexType, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_TYPE_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE)

#undef REG_TYPE_NODE

#define REG_TYPE_NODE(Name, Suffix) Name##Suffix,

    using TypeNode = std::variant<TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE, Node) std::monostate>;
    using TypeNodePtr = std::variant<TINY_COBALT_AST_TYPE_NODES(REG_TYPE_NODE, Ptr) std::nullptr_t>;

#undef REG_TYPE_NODE

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
            templateName(std::move(templateName)),
            templateArgs(std::move(templateArgs)) {}
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_TYPENODE_H_