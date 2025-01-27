//
// Created by Renatus Madrigal on 01/27/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_

#include <memory>
#include <proxy.h>
#include <vector>
#include "AST/ASTVisitor.h"
#include "Semantic/DeclMatcher.h"
#include "Semantic/TypeAnalyzer.h"

namespace TinyCobalt::Semantic {

    /**
     * A class to perform semantic analysis on the AST.
     */
    class SemanticAnalyzer : public AST::BaseASTVisitor<SemanticAnalyzer> {
    public:
        SemanticAnalyzer() {
            middlewares_.push_back(std::make_unique<DeclMatcher>());
            middlewares_.push_back(std::make_unique<TypeAnalyzer>());
        }

        SemanticAnalyzer(const SemanticAnalyzer &) = delete;
        SemanticAnalyzer &operator=(const SemanticAnalyzer &) = delete;

    private:
        std::vector<AST::ASTVisitorMiddlewarePtr> middlewares_;
    };

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_SEMANTICANALYZER_H_