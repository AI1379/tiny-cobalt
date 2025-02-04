//
// Created by Renatus Madrigal on 01/26/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_DECLMATCHER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_DECLMATCHER_H_

#include <string>
#include "AST/ASTNode.h"
#include "AST/ASTNodeDecl.h"
#include "AST/ASTVisitor.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Assert.h"
#include "Semantic/Scope.h"

namespace TinyCobalt::Semantic {

    class DeclMatcher {
    public:
        DeclMatcher() { pushScope(); }

        ~DeclMatcher() {
            delete current_alias_;
            delete current_func_;
            delete current_variable_;
            delete current_struct_;
        }

        AST::VisitorState beforeSubtree(AST::ASTNodePtr node);
        AST::VisitorState beforeChild(AST::ASTNodePtr node, AST::ASTNodePtr child);
        AST::VisitorState afterChild(AST::ASTNodePtr node, AST::ASTNodePtr child);

    private:
        using FuncScope = Scope<std::string, AST::FuncDefPtr>;
        using VariableScope = Scope<std::string, AST::VariableDefPtr>;
        using AliasScope = Scope<std::string, AST::AliasDefPtr>;
        using StructScope = Scope<std::string, AST::StructDefPtr>;

        void pushScope(const std::string &name = kDefaultScopeName);

        void popScope();

        using TypeDefPtr = AST::SimpleTypeNode::TypeDefPtr;

        TypeDefPtr findType(const std::string &name);

        Scope<std::string, AST::FuncDefPtr> *current_func_ = nullptr;
        Scope<std::string, AST::VariableDefPtr> *current_variable_ = nullptr;
        Scope<std::string, AST::AliasDefPtr> *current_alias_ = nullptr;
        Scope<std::string, AST::StructDefPtr> *current_struct_ = nullptr;
    };

    TINY_COBALT_CONCEPT_ASSERT(AST::ASTVisitorMiddlewareConcept, DeclMatcher);

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_DECLMATCHER_H_