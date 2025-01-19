//
// Created by Renatus Madrigal on 01/19/2025
//

#include <gtest/gtest.h>
#include "AST/AST.h"
#include "AST/ASTNodeDecl.h"
#include "AST/ExprNode.h"
#include "Common/JSON.h"

using namespace TinyCobalt::AST;
using TinyCobalt::Common::JSON;

TEST(ASTNode, ExprNode1) {
    auto const_expr = std::make_shared<ConstExprNode>("1", ConstExprType::Int);
    auto variable = std::make_shared<VariableNode>("x");
    auto binary1 = std::make_shared<BinaryNode>(const_expr, BinaryOp::Mul, const_expr);
    auto binary2 = std::make_shared<BinaryNode>(variable, BinaryOp::Add, binary1);
    auto stmt = std::make_shared<ExprStmtNode>(binary2);
    ASTNodePtr ast = stmt;
    JSON exp = {
            {"type", "ExprStmt"},
            {"expr",
             {
                     {"type", "Binary"},
                     {"op", "Add"},
                     {"lhs",
                      {
                              {"type", "Variable"},
                              {"name", "x"},
                      }},
                     {"rhs",
                      {
                              {"type", "Binary"},
                              {"op", "Mul"},
                              {"lhs",
                               {
                                       {"type", "ConstExpr"},
                                       {"value", "1"},
                                       {"expr_type", "Int"},
                               }},
                              {"rhs",
                               {
                                       {"type", "ConstExpr"},
                                       {"value", "1"},
                                       {"expr_type", "Int"},
                               }},
                      }},
             }},
    };
    JSON res = ast->toJSON();
    EXPECT_EQ(res, exp);
}
