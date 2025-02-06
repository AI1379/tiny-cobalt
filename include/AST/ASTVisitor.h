//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"
#include "Common/Utility.h"

#include <concepts>
#include <proxy.h>
#include <type_traits>

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
          ::add_convention<MemBeforeSubtree, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<MemAfterSubtree, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<MemBeforeChild, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::add_convention<MemAfterChild, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorMiddlewareConcept =
            pro::proxiable<T *, ASTVisitorMiddlewareProxy> && std::is_default_constructible_v<T>;

    using ASTVisitorMiddlewarePtr = pro::proxy<ASTVisitorMiddlewareProxy>;

    // TODO: Use CRTP to auto forward ASTNodePtr to detailed node pointer
    template<typename Middleware>
    class BaseASTVisitorMiddleware {
    public:
        // INTERFACES
        VisitorState beforeSubtree(this Middleware self, ASTNodePtr node) {
            if constexpr (requires(Middleware M) {
                              { M.beforeSubtreeImpl(node) } -> std::same_as<VisitorState>;
                          }) {
                return self.beforeSubtreeImpl(node);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState afterSubtree(this Middleware self, ASTNodePtr node) {
            if constexpr (requires(Middleware M) {
                              { M.afterSubtreeImpl(node) } -> std::same_as<VisitorState>;
                          }) {
                return self.afterSubtreeImpl(node);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState beforeChild(this Middleware self, ASTNodePtr node, ASTNodePtr child) {
            if constexpr (requires(Middleware M) {
                              { M.beforeChildImpl(node, child) } -> std::same_as<VisitorState>;
                          }) {
                return self.beforeChildImpl(node, child);
            } else {
                return VisitorState::Normal;
            }
        }

        VisitorState afterChild(this Middleware self, ASTNodePtr node, ASTNodePtr child) {
            if constexpr (requires(Middleware M) {
                              { M.afterChildImpl(node, child) } -> std::same_as<VisitorState>;
                          }) {
                return self.afterChildImpl(node, child);
            } else {
                return VisitorState::Normal;
            }
        }

        friend Middleware;
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
                if (this->visit(child) == VisitorState::Exit) {
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
        Middleware middleware_{};
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
