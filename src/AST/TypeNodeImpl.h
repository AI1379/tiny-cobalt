//
// Created by Renatus Madrigal on 12/12/2024
//

#ifndef TINY_COBALT_SRC_AST_TYPENODEIMPL_H_
#define TINY_COBALT_SRC_AST_TYPENODEIMPL_H_

#include "AST/ASTNode.h"
#include "AST/TypeNode.h"
#include "Common/JSON.h"

#include <proxy.h>
#include <vector>

namespace TinyCobalt::AST {
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

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_SRC_AST_TYPENODEIMPL_H_