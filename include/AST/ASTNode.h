//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cassert>
#include <memory>
#include <ostream>
#include <proxy.h>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Generator.h"
#include "Common/JSON.h"


#define TINY_COBALT_AST_NODES(X, ...)                                                                                  \
    TINY_COBALT_AST_EXPR_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_STMT_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_TYPE_NODES(X, __VA_ARGS__)                                                                         \
    X(ASTRoot, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_NODE_EQ(Name, ...) bool operator==(const Name##Ptr &, const Name##Ptr &);

    TINY_COBALT_AST_EXPR_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_STMT_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_TYPE_NODES(REG_NODE_EQ)

#undef REG_NODE_EQ

    PRO_DEF_MEM_DISPATCH(MemTraverse, traverse);
    PRO_DEF_MEM_DISPATCH(MemThisPointer, thisPointer);

    // TODO: Check if it is possible not to use RTTI
    struct ContainTypeRefl {
        template<typename T>
        constexpr explicit ContainTypeRefl(std::in_place_type_t<T>) : type_(typeid(T)) {}

        template<typename F, typename R>
        struct accessor {
            template<typename T>
            bool containType() const noexcept {
                const ContainTypeRefl &self = pro::proxy_reflect<R>(pro::access_proxy<F>(*this));
                return self.type_.hash_code() == typeid(T).hash_code();
            }

            template<typename T>
            std::shared_ptr<T> cast() const noexcept {
                assert(containType<T>());
                // FIXME: No restriction on the type of F
                return reinterpret_cast<T *>(pro::access_proxy<F>(*this)->thisPointer())->shared_from_this();
            }
        };

    private:
        const std::type_info &type_;
    };


    struct ASTNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_reflection<ContainTypeRefl> // NOLINT
          ::add_convention<MemTraverse, Utility::Generator<pro::proxy<ASTNodeProxy>>()> // NOLINT
          // FIXME: erased type information may lead to memory leaks
          ::add_convention<MemThisPointer, void *() const> // NOLINT
          ::add_facade<Common::ToJSONProxy> // NOLINT
          ::support_copy<pro::constraint_level::nontrivial> // NOLINT
          ::build {};


    template<typename T>
    struct EnableThisPointer : public std::enable_shared_from_this<T> {
        void *thisPointer() const { return const_cast<void *>(reinterpret_cast<const void *>(this)); }
    };

    template<typename T>
    concept ASTNodeConcept = pro::proxiable<T *, ASTNodeProxy>;

    template<typename T>
    concept ASTNodePtrConcept = pro::proxiable<T, ASTNodeProxy> || std::is_convertible_v<T, pro::proxy<ASTNodeProxy>>;

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;

    struct TypeNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTNodeProxy, true> // NOLINT
          ::add_convention<MemConvertibleTo, bool(const pro::proxy<TypeNodeProxy> &) const> // NOLINT
          ::build {};

    template<typename T>
    concept TypeNodeConcept = pro::proxiable<T *, TypeNodeProxy>;
    template<typename T>
    concept TypeNodePtrConcept = pro::proxiable<T, TypeNodeProxy>;
    using TypeNodePtr = pro::proxy<TypeNodeProxy>;

    static_assert(ASTNodePtrConcept<TypeNodePtr>, "TypeNodePtr is not an ASTNodePtr");

    struct ExprNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTNodeProxy, true> // NOLINT
          ::add_convention<MemEvalType, AST::TypeNodePtr()> // NOLINT
          ::build {};

    template<typename T>
    concept ExprNodeConcept = pro::proxiable<T *, ExprNodeProxy>;
    template<typename T>
    concept ExprNodePtrConcept = pro::proxiable<T, ExprNodeProxy>;
    using ExprNodePtr = pro::proxy<ExprNodeProxy>;

    static_assert(ASTNodePtrConcept<ExprNodePtr>, "ExprNodePtr is not an ASTNodePtr");

    struct StmtNodeProxy // NOLINT
        : pro::facade_builder // NOLINT
          ::add_facade<ASTNodeProxy, true> // NOLINT
          ::add_convention<MemStmtFlag, void()> // NOLINT
          ::build {};

    template<typename T>
    concept StmtNodeConcept = pro::proxiable<T *, StmtNodeProxy>;
    template<typename T>
    concept StmtNodePtrConcept = pro::proxiable<T, StmtNodeProxy>;
    using StmtNodePtr = pro::proxy<StmtNodeProxy>;

    static_assert(ASTNodePtrConcept<StmtNodePtr>, "StmtNodePtr is not an ASTNodePtr");

    struct ASTRootNode : public EnableThisPointer<ASTRootNode> {
        std::vector<ASTNodePtr> children;
        explicit ASTRootNode(std::vector<ASTNodePtr> children) : children(std::move(children)) {}
        template<typename T>
            requires ASTNodePtrConcept<T>
        explicit ASTRootNode(std::vector<T> children) {
            for (const auto &child: children) {
                this->children.push_back(child);
            }
        }
        ASTNodeGen traverse() {
            for (const auto &child: children) {
                co_yield child;
            }
        }
        Common::JSON toJSON() const {
            Common::JSON json;
            json["type"] = "ASTRoot";
            json["children"] = Common::JSON::array();
            for (const auto &child: children) {
                json["children"].push_back(child->toJSON());
            }
            return json;
        }
    };
    using ASTRootPtr = std::shared_ptr<ASTRootNode>;

    static_assert(ASTNodePtrConcept<ASTRootPtr>, "ASTNodePtrConcept<ASTRootPtr> is not satisfied.");

    inline std::ostream &operator<<(std::ostream &os, const ASTNodePtr &node) { return os << node->toJSON(); }

    template<typename T>
        requires ASTNodePtrConcept<T>
    inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &nodes) {
        os << "[";
        for (const auto &node: nodes)
            os << node << ",";
        os << "]";
        return os;
    }

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_