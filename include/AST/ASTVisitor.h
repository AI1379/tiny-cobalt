//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"
#include "Common/Utility.h"

#include <concepts>
#include <proxy.h>

namespace TinyCobalt::AST {

    enum class VisitorState {
        EmptyNode = 0,
        Normal,
        Continue,
        Break,
        Exit,
    };

    PRO_DEF_MEM_DISPATCH(MemBeforeSubtree, beforeSubtree);
    PRO_DEF_MEM_DISPATCH(MemAfterSubtree, afterSubtree);
    // WARNING: It is not supposed to use MemBeforeChild and MemAfterChild directly
    // Traverse child nodes in MemBeforeSubtree and MemAfterSubtree is more recommended
    PRO_DEF_MEM_DISPATCH(MemBeforeChild, beforeChild);
    PRO_DEF_MEM_DISPATCH(MemAfterChild, afterChild);

    PRO_DEF_MEM_DISPATCH(MemVisit, visit);

    struct ASTVisitorMiddlewareProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<pro::weak_dispatch<MemBeforeSubtree>, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<pro::weak_dispatch<MemAfterSubtree>, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<pro::weak_dispatch<MemBeforeChild>, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::add_convention<pro::weak_dispatch<MemAfterChild>, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::add_reflection<WeakImplementedRefl<ASTVisitorMiddlewareProxy>> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorMiddlewareConcept = pro::proxiable<T *, ASTVisitorMiddlewareProxy>;

    using ASTVisitorMiddlewarePtr = pro::proxy<ASTVisitorMiddlewareProxy>;

    // TODO: Use CRTP to auto forward ASTNodePtr to detailed node pointer
    template<typename Middleware>
    class BaseASTVisitorMiddleware {
    public:
        // INTERFACES
        VisitorState beforeSubtree(this Middleware self, ASTNodePtr node) {
            if constexpr (requires(Middleware M) {
                              { M.beforeSubtree(node) } -> std::same_as<VisitorState>;
                          }) {
                return self.beforeSubtree(node);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState afterSubtree(this Middleware self, ASTNodePtr node) {
            if constexpr (requires(Middleware M) {
                              { M.afterSubtree(node) } -> std::same_as<VisitorState>;
                          }) {
                return self.afterSubtree(node);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState beforeChild(this Middleware self, ASTNodePtr node, ASTNodePtr child) {
            if constexpr (requires(Middleware M) {
                              { M.beforeChild(node, child) } -> std::same_as<VisitorState>;
                          }) {
                return self.beforeChild(node, child);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState afterChild(this Middleware self, ASTNodePtr node, ASTNodePtr child) {
            if constexpr (requires(Middleware M) {
                              { M.afterChild(node, child) } -> std::same_as<VisitorState>;
                          }) {
                return self.afterChild(node, child);
            } else {
                return VisitorState::Normal;
            }
        }
    };

    struct ASTVisitorProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<MemVisit, VisitorState(ASTNodePtr)> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorConcept = pro::proxiable<T *, ASTVisitorProxy>;

    // We use CRTP to implement the AST visitor
    template<typename Middleware>
        requires ASTVisitorMiddlewareConcept<Middleware>
    class BaseASTVisitor {
    public:
        VisitorState visit(ASTNodePtr node) {
            if (!node)
                return VisitorState::EmptyNode;
            switch (middleware_.beforeSubtree(node)) {
                case VisitorState::Break:
                    return VisitorState::Normal;
                case VisitorState::Exit:
                    return VisitorState::Exit;
                default:
                    break;
            }
            for (auto child: node->traverse()) {
                if (!child)
                    continue;
                switch (middleware_.beforeChild(node, child)) {
                    case VisitorState::Continue:
                        continue;
                    case VisitorState::Break:
                        goto BreakTag;
                    case VisitorState::Exit:
                        return VisitorState::Exit;
                    default:
                        break;
                }
                if (middleware_.visit(child) == VisitorState::Exit) {
                    return VisitorState::Exit;
                }
                switch (middleware_.afterChild(node, child)) {
                    case VisitorState::Continue:
                        continue;
                    case VisitorState::Break:
                        goto BreakTag;
                    case VisitorState::Exit:
                        return VisitorState::Exit;
                    default:
                        break;
                }
            }
        BreakTag:
            switch (middleware_.afterSubtree(node)) {
                case VisitorState::Exit:
                    return VisitorState::Exit;
                default:
                    return VisitorState::Normal;
            }
        }

    private:
        Middleware middleware_;
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
