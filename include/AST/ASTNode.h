//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cassert>
#include <iostream>
#include <memory>
#include <proxy.h>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include "Common/Generator.h"
#include "Common/JSON.h"


#define TINY_COBALT_AST_NODES(X, ...)                                                                                  \
    TINY_COBALT_AST_EXPR_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_STMT_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_TYPE_NODES(X, __VA_ARGS__)                                                                         \
    X(ASTRoot, __VA_ARGS__)

namespace TinyCobalt::AST {

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
          // TODO: rewrite toJSON() using generator.
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