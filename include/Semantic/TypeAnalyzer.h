//
// Created by Renatus Madrigal on 01/27/2025
//

#ifndef TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_
#define TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_

#include "AST/ASTVisitor.h"
#include "Common/Assert.h"
namespace TinyCobalt::Semantic {
    class TypeAnalyzer {};

    TINY_COBALT_CONCEPT_ASSERT(AST::ASTVisitorMiddlewareConcept, TypeAnalyzer);

} // namespace TinyCobalt::Semantic

#endif // TINY_COBALT_INCLUDE_SEMANTIC_TYPEANALYZER_H_