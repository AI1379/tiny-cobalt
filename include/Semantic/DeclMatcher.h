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

    class DeclMatcher : public AST::BaseASTVisitorMiddleware<DeclMatcher> {
    public:
        DeclMatcher() { pushScope("<root>"); }

        ~DeclMatcher() {}

        AST::VisitorState beforeSubtreeImpl(AST::ASTNodePtr node);
        AST::VisitorState beforeChildImpl(AST::ASTNodePtr node, AST::ASTNodePtr child);
        AST::VisitorState afterChildImpl(AST::ASTNodePtr node, AST::ASTNodePtr child);

    private:
        using FuncScope = Scope<std::string, AST::FuncDefPtr>;
        using VariableScope = Scope<std::string, AST::VariableDefPtr>;
        using AliasScope = Scope<std::string, AST::AliasDefPtr>;
        using StructScope = Scope<std::string, AST::StructDefPtr>;

        void pushScope(const std::string &name = kDefaultScopeName);

        void popScope();

        using TypeDefPtr = AST::SimpleTypeNode::TypeDefPtr;

        TypeDefPtr findType(const std::string &name);

        std::shared_ptr<FuncScope> current_func_ = nullptr;
        std::shared_ptr<VariableScope> current_variable_ = nullptr;
        std::shared_ptr<AliasScope> current_alias_ = nullptr;
        std::shared_ptr<StructScope> current_struct_ = nullptr;

        void tryAddSymbol(AST::FuncDefPtr ptr);
        void tryAddSymbol(AST::VariableDefPtr ptr);
        void tryAddSymbol(AST::AliasDefPtr ptr);
        void tryAddSymbol(AST::StructDefPtr ptr);
    };

    TINY_COBALT_CONCEPT_ASSERT(AST::ASTVisitorMiddlewareConcept, DeclMatcher);

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_DECLMATCHER_H_