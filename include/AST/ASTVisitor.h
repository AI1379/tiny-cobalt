//
// Created by Renatus Madrigal on 12/19/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
#define TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_

#include "AST/ASTNode.h"
#include "Common/Utility.h"

#include <proxy.h>
#include <tuple>

namespace TinyCobalt::AST {

    enum class VisitorState {
        EmptyNode = 0,
        Normal,
        Continue,
        Break,
        Exit,
    };

    template<typename... Args>
    VisitorState EmptyVisit(auto &&...) {
        return VisitorState::Normal;
    }

    PRO_DEF_MEM_DISPATCH(MemBeforeSubtree, beforeSubtree);
    PRO_DEF_WEAK_DISPATCH(WeakMemBeforeSubtree, MemBeforeSubtree, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemAfterSubtree, afterSubtree);
    PRO_DEF_WEAK_DISPATCH(WeakMemAfterSubtree, MemAfterSubtree, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemBeforeChild, beforeChild);
    PRO_DEF_WEAK_DISPATCH(WeakMemBeforeChild, MemBeforeChild, EmptyVisit);
    PRO_DEF_MEM_DISPATCH(MemAfterChild, afterChild);
    PRO_DEF_WEAK_DISPATCH(WeakMemAfterChild, MemAfterChild, EmptyVisit);

    PRO_DEF_MEM_DISPATCH(MemVisit, visit);

    struct ASTVisitorMiddlewareProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_convention<WeakMemBeforeSubtree, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemAfterSubtree, VisitorState(ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemBeforeChild, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::add_convention<WeakMemAfterChild, VisitorState(ASTNodePtr, ASTNodePtr)> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorMiddlewareConcept = pro::proxiable<T *, ASTVisitorMiddlewareProxy>;

    using ASTVisitorMiddlewarePtr = pro::proxy<ASTVisitorMiddlewareProxy>;

    struct ASTVisitorProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTVisitorMiddlewareProxy> // NOLINT
          ::add_convention<MemVisit, VisitorState(ASTNodePtr)> // NOLINT
          ::build {};

    template<typename T>
    concept ASTVisitorConcept = pro::proxiable<T *, ASTVisitorProxy>;

    // We use CRTP to implement the AST visitor
    template<typename VisitorImpl>
        requires ASTVisitorMiddlewareConcept<VisitorImpl>
    class BaseASTVisitor {
    public:
        // INTERFACES
        VisitorState beforeSubtree(ASTNodePtr node) { return VisitorState::Normal; }
        VisitorState afterSubtree(ASTNodePtr node) { return VisitorState::Normal; }
        VisitorState beforeChild(ASTNodePtr node, ASTNodePtr child) { return VisitorState::Normal; }
        VisitorState afterChild(ASTNodePtr node, ASTNodePtr child) { return VisitorState::Normal; }


        VisitorState visit(this VisitorImpl &self, ASTNodePtr node) {
            if (!node)
                return VisitorState::EmptyNode;
            switch (self.beforeSubtree(node)) {
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
                switch (self.beforeChild(node, child)) {
                    case VisitorState::Continue:
                        continue;
                    case VisitorState::Break:
                        goto BreakTag;
                    case VisitorState::Exit:
                        return VisitorState::Exit;
                    default:
                        break;
                }
                if (self.visit(child) == VisitorState::Exit) {
                    return VisitorState::Exit;
                }
                switch (self.afterChild(node, child)) {
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
            switch (self.afterSubtree(node)) {
                case VisitorState::Exit:
                    return VisitorState::Exit;
                default:
                    return VisitorState::Normal;
            }
        }
    };

    namespace detail {
        template<typename T>
        auto subtreeInvokeHelper(T &&func, ASTNodePtr node) {
            using Arg = std::tuple_element_t<0, typename function_traits<T>::args_type>;
            if (node->containType<Arg>()) {
                return func(node->cast<Arg>());
            } else {
                return VisitorState::EmptyNode;
            }
        }

        template<typename T>
        auto childInvokeHelper(T &&func, ASTNodePtr node, ASTNodePtr child) {
            using Arg1 = std::tuple_element_t<0, typename function_traits<T>::args_type>;
            using Arg2 = std::tuple_element_t<1, typename function_traits<T>::args_type>;
            if (node->containType<Arg1>() && child->containType<Arg2>()) {
                return func(node->cast<Arg1>(), child->cast<Arg2>());
            } else {
                return VisitorState::EmptyNode;
            }
        }
    } // namespace detail

    template<typename... Ts>
    struct BeforeSubtreeVisitorPack : Ts... {
        using Ts::operator()...;
        VisitorState beforeSubtree(ASTNodePtr node) {
            return (detail::subtreeInvokeHelper<Ts>(static_cast<Ts>(*this), node) | ...);
        }
    };

    template<typename... Ts>
    struct AfterSubtreeVisitorPack : Ts... {
        using Ts::operator()...;
        VisitorState afterSubtree(ASTNodePtr node) {
            return (detail::subtreeInvokeHelper<Ts>(static_cast<Ts>(*this), node) | ...);
        }
    };

    template<typename... Ts>
    struct BeforeChildVisitorPack : Ts... {
        using Ts::operator()...;
        VisitorState beforeChild(ASTNodePtr node, ASTNodePtr child) {
            return (detail::childInvokeHelper<Ts>(static_cast<Ts>(*this), node, child) | ...);
        }
    };

    template<typename... Ts>
    struct AfterChildVisitorPack : Ts... {
        using Ts::operator()...;
        VisitorState afterChild(ASTNodePtr node, ASTNodePtr child) {
            return (detail::childInvokeHelper<Ts>(static_cast<Ts>(*this), node, child) | ...);
        }
    };

    template<typename... Ts>
    struct VisitorPack : Ts... {
        using Ts::beforeSubtree...;
        using Ts::afterSubtree...;
        using Ts::beforeChild...;
        using Ts::afterChild...;
    };
} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_ASTVISITOR_H_
