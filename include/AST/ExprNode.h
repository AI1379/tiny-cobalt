//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_EXPRNODE_H_
#define TINY_COBALT_INCLUDE_AST_EXPRNODE_H_

#include <cstddef>
#include <memory>
#include <variant>

#define TINY_COBALT_AST_EXPR_NODES(X, ...)                                                                             \
    X(ConstExpr, __VA_ARGS__)                                                                                          \
    X(Assign, __VA_ARGS__)                                                                                             \
    X(Binary, __VA_ARGS__)                                                                                             \
    X(Unary, __VA_ARGS__)                                                                                              \
    X(Cast, __VA_ARGS__)                                                                                               \
    X(Condition, __VA_ARGS__)                                                                                          \
    X(FuncCall, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_EXPR_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_EXPR_NODES(REG_EXPR_NODE)

#undef REG_EXPR_NODE

// #define REG_EXPR_NODE(Name, Suffix) Name##Suffix,

//     using ExprNode = std::variant<TINY_COBALT_AST_EXPR_NODES(REG_EXPR_NODE, Node) std::monostate>;
//     using ExprNodePtr = std::variant<TINY_COBALT_AST_EXPR_NODES(REG_EXPR_NODE, Ptr) std::nullptr_t>;

// #undef REG_EXPR_NODE

    // clang-format off
    enum class UnaryOp {
        Plus,
        Minus,
        Not,
        BitNot,
        PreInc,
        PreDec,
        PostInc,
        PostDec,
        Address,
        Deref
    };
    // clang-format on

    enum class BinaryOp {
        Add,
        Sub,
        Mul,
        Div,
        Mod,
        BitAnd,
        BitOr,
        BitXor,
        BitLShift,
        BitRShift,
        And,
        Or,
        Eq,
        Ne,
        Lt,
        Gt,
        Le,
        Ge,
        Assign,
        AddAssign,
        SubAssign,
        MulAssign,
        DivAssign,
        ModAssign,
        BitAndAssign,
        BitOrAssign,
        BitXorAssign,
        BitLShiftAssign,
        BitRShiftAssign,
        Member,
        PtrMember,
    };

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_EXPRNODE_H_