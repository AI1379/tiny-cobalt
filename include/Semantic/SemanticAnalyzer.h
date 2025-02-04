//
// Created by Renatus Madrigal on 01/27/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_

#include "AST/ASTVisitor.h"

namespace TinyCobalt::Semantic {

    namespace detail {
        class SemanticAnalyzerMiddleware : AST::BaseASTVisitorMiddleware<SemanticAnalyzerMiddleware> {
            // TODO: Implement the semantic analyzer middleware
        };
    } // namespace detail

    /**
     * A class to perform semantic analysis on the AST.
     */
    using SemanticAnalyzer = AST::BaseASTVisitor<detail::SemanticAnalyzerMiddleware>;

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_