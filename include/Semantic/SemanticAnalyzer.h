//
// Created by Renatus Madrigal on 01/27/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_

#include "AST/ASTNode.h"
#include "AST/ASTVisitor.h"
#include "Common/Assert.h"
#include "Semantic/DeclMatcher.h"
#include "Semantic/TypeAnalyzer.h"

namespace TinyCobalt::Semantic {

    namespace detail {
        class SemanticAnalyzerMiddleware : AST::BaseASTVisitorMiddleware<SemanticAnalyzerMiddleware> {
        public:
            SemanticAnalyzerMiddleware() = default;

            AST::VisitorState beforeSubtree(AST::ASTNodePtr node) { return decl_matcher_.beforeSubtree(node); }

            AST::VisitorState beforeChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
                return decl_matcher_.beforeChild(node, node);
            }

            AST::VisitorState afterChild(AST::ASTNodePtr node, AST::ASTNodePtr child) {
                return decl_matcher_.afterChild(node, child);
            }

            AST::VisitorState afterSubtree(AST::ASTNodePtr node) { return type_analyzer_.afterSubtree(node); }

        private:
            DeclMatcher decl_matcher_;
            TypeAnalyzer type_analyzer_;
        };

        TINY_COBALT_CONCEPT_ASSERT(AST::ASTVisitorMiddlewareConcept, SemanticAnalyzerMiddleware);
    } // namespace detail

    /**
     * A class to perform semantic analysis on the AST.
     */
    // using SemanticAnalyzer = AST::BaseASTVisitor<detail::SemanticAnalyzerMiddleware>;

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_