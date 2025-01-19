//
// Created by Renatus Madrigal on 12/11/2024
//

#ifndef TINY_COBALT_INCLUDE_AST_EXPRNODE_H_
#define TINY_COBALT_INCLUDE_AST_EXPRNODE_H_

#include <memory>
#include <proxy.h>

#define TINY_COBALT_AST_EXPR_NODES(X, ...)                                                                             \
    X(ConstExpr, __VA_ARGS__)                                                                                          \
    X(Variable, __VA_ARGS__)                                                                                           \
    X(Binary, __VA_ARGS__)                                                                                             \
    X(Unary, __VA_ARGS__)                                                                                              \
    X(Multiary, __VA_ARGS__)                                                                                           \
    X(Cast, __VA_ARGS__)                                                                                               \
    X(Condition, __VA_ARGS__)

namespace TinyCobalt::AST {

#define REG_EXPR_NODE(Name, ...)                                                                                       \
    struct Name##Node;                                                                                                 \
    using Name##Ptr = std::shared_ptr<Name##Node>;

    TINY_COBALT_AST_EXPR_NODES(REG_EXPR_NODE)

#undef REG_EXPR_NODE

    enum class UnaryOp {
        Positive,
        Negative,
        Not,
        BitNot,
        PreInc,
        PreDec,
        PostInc,
        PostDec,
        Addr,
        Deref,
    };

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
        Less,
        Greater,
        Leq,
        Geq,
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

    enum class MultiaryOp {
        Subscript,
        FuncCall,
        Comma,
    };

    enum class ConstExprType {
        Int,
        HexInt,
        OctInt,
        BinInt,
        Float,
        String,
        Char,
        Bool,
    };

    enum class CastType {
        Static,
        Reinterpret,
        Const,
    };

    PRO_DEF_MEM_DISPATCH(MemEvalType, evalType);

} // namespace TinyCobalt::AST

#endif // TINY_COBALT_INCLUDE_AST_EXPRNODE_H_