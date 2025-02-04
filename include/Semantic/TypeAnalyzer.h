//
// Created by Renatus Madrigal on 01/27/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_

#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/TypeNode.h"
#include "Common/Assert.h"

namespace TinyCobalt::Semantic {
    class TypeAnalyzer : public AST::BaseASTVisitorMiddleware<TypeAnalyzer> {
    public:
        AST::VisitorState afterSubtree(AST::ASTNodePtr node);

    private:
#define REG_ANALYZE_NODE(Name, ...) AST::TypeNodePtr analyzeType(AST::Name##Ptr node);
        TINY_COBALT_AST_EXPR_NODES(REG_ANALYZE_NODE)
#undef REG_ANALYZE_NODE
    };

    TINY_COBALT_CONCEPT_ASSERT(AST::ASTVisitorMiddlewareConcept, TypeAnalyzer);

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_