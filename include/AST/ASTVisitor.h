//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"

#include <proxy.h>

namespace TinyCobalt::AST {

    template<typename... Args>
    void EmptyVisit(auto &&...) {}

    PRO_DEF_MEM_DISPATCH(MemBeforeSubtree, beforeSubtree);
    PRO_DEF_WEAK_DISPATCH(WeakMemBeforeSubtree, MemBeforeSubtree, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemAfterSubtree, afterSubtree);
    PRO_DEF_WEAK_DISPATCH(WeakMemAfterSubtree, MemAfterSubtree, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemBeforeChild, beforeChild);
    PRO_DEF_WEAK_DISPATCH(WeakMemBeforeChild, MemBeforeChild, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemAfterChild, afterChild);
    PRO_DEF_WEAK_DISPATCH(WeakMemAfterChild, MemAfterChild, EmptyVisit);

    struct ASTVisitorProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<WeakMemBeforeSubtree, void(ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemAfterSubtree, void(ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemBeforeChild, void(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemAfterChild, void(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorConcept = pro::proxiable<T *, ASTVisitorProxy>;

    // We use CRTP to implement the AST visitor
    template<typename VisitorImpl>
        requires ASTVisitorConcept<VisitorImpl>
    class BaseASTVisitor {
    public:
        // INTERFACES
        void beforeSubtree(ASTNodePtr node) {}
        void afterSubtree(ASTNodePtr node) {}
        void beforeChild(ASTNodePtr node, ASTNodePtr child) {}
        void afterChild(ASTNodePtr node, ASTNodePtr child) {}


        void visit(this VisitorImpl &self, ASTNodePtr node) {
            if (node)
                self.beforeSubtree(node);
            if (self.broke) {
                self.broke = false;
                return;
            }
            for (auto child: node->traverse()) {
                if (!child)
                    continue;
                self.beforeChild(node, child);
                if (self.broke) {
                    self.broke = false;
                    break;
                }
                if (self.continued) {
                    self.continued = false;
                    continue;
                }
                self.visit(child);
                self.afterChild(node, child);
            }
            if (node)
                self.afterSubtree(node);
        }

    protected:
        bool broke = false, continued = false;
        void break_() { broke = true; }
        void continue_() { continued = true; }
    };
} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
