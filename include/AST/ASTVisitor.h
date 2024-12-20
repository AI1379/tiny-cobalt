//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"

#include <proxy.h>

namespace TinyCobalt::AST {

    template<typename VisitorImpl>
    class ASTVisitor {
    public:
        // TODO: down cast TraverseablePtr to specific Node Pointer using template or overload
        // INTEERFACES
        void beforeSubtree(ASTNodePtr node) {}
        void afterSubtree(ASTNodePtr node) {}
        void beforeChild(ASTNodePtr node, ASTNodePtr child) {}
        void afterChild(ASTNodePtr node, ASTNodePtr child) {}


        void visit(this VisitorImpl &self, ASTNodePtr node) {
            if (node)
                self.beforeSubtree(node);
            if (self.breaked) {
                self.breaked = false;
                return;
            }
            // TODO: let node->traverse return a enum class to indicate the type of child node
            for (auto child: node->traverse()) {
                if (!child)
                    continue;
                self.beforeChild(node, child);
                if (self.breaked) {
                    self.breaked = false;
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
        bool breaked, continued;
        void break_() { breaked = true; }
        void continue_() { continued = true; }
    };
} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
