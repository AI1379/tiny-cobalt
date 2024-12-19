//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"

#include <proxy.h>

namespace TinyCobalt::AST {

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);

    struct TraverseableProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<TraverseableProxy>>()> // NOLINT
          ::build {};

    template<typename T>
    concept Traverseable = pro::proxiable<T *, TraverseableProxy>;

    using TraverseablePtr = pro::proxy<TraverseableProxy>;
    using TraverseableGen = Utility::Generator<TraverseablePtr>;

    template<typename VisitorImpl>
    class ASTVisitor {
    public:
        // TODO: down cast TraverseablePtr to specific Node Pointer using template or overload
        // INTEERFACES
        void beforeSubtree(TraverseablePtr node) {}
        void afterSubtree(TraverseablePtr node) {}
        void beforeChild(TraverseablePtr node, TraverseablePtr child) {}
        void afterChild(TraverseablePtr node, TraverseablePtr child) {}


        void visit(this VisitorImpl &self, TraverseablePtr node) {
            if (node)
                self.beforeSubtree(node);
            if (self.breaked) {
                self.breaked = false;
                return;
            }
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
