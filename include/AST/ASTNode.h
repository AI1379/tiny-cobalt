//
// Created by Renatus Madrigal on 12/10/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_
#define TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_

#include <cassert>
#include <memory>
#include <proxy.h>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "Common/Generator.h"


#define TINY_COBALT_AST_NODES(X, ...)                                                                                  \
    TINY_COBALT_AST_EXPR_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_STMT_NODES(X, __VA_ARGS__)                                                                         \
    TINY_COBALT_AST_TYPE_NODES(X, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_NODE_EQ(Name, ...) bool operator==(const Name##Ptr &, const Name##Ptr &);

    TINY_COBALT_AST_EXPR_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_STMT_NODES(REG_NODE_EQ)
    TINY_COBALT_AST_TYPE_NODES(REG_NODE_EQ)
    // REG_NODE_EQ(ExprNode, )
    // REG_NODE_EQ(StmtNode, )
    // REG_NODE_EQ(TypeNode, )

#undef REG_NODE_EQ

    // using ASTNode = Utility::UnionedVariant<ExprNode, StmtNode, TypeNode>;
    // using ASTNodePtr = Utility::UnionedVariant<ExprNodePtr, StmtNodePtr, TypeNodePtr>;

    // FIXME: There may be memory leaks

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
          ::support_copy<pro::constraint_level::nontrivial> // NOLINT
          ::build {};

    template<typename T>
    struct EnableThisPointer : public std::enable_shared_from_this<T> {
        void *thisPointer() const { return const_cast<void *>(reinterpret_cast<const void *>(this)); }
    };

    template<typename T>
    concept ASTNodeConcept = pro::proxiable<T *, ASTNodeProxy>;

    template<typename T>
    concept ASTNodePtrConcept = pro::proxiable<T, ASTNodeProxy>;

    using ASTNodePtr = pro::proxy<ASTNodeProxy>;
    using ASTNodeGen = Utility::Generator<ASTNodePtr>;

    // TODO: use more strict proxy to restrict these types
    using ExprNodePtr = ASTNodePtr;
    using StmtNodePtr = ASTNodePtr;
    using TypeNodePtr = ASTNodePtr;

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_BASEASTNODE_H_