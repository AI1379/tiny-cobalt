//
// Created by Renatus Madrigal on 01/18/2025
//

#include <exception>
#include <gtest/gtest.h>
#include <sstream>

#include "Common/JSON.h"
#include "LexerParser/Parser.h"

using namespace TinyCobalt;

TEST(LexerParser, BinaryExpr1) {
    LexerParser::Parser parser;
    std::string input = R"(1 + 2 * 3;)";
    std::istringstream is(input);
    std::ostringstream os;
    parser.switchInput(&is).switchOutput(&os);
    auto err = parser.parse();
    ASSERT_EQ(err, 0);
    auto res = parser.result();
    Common::JSON expected = {{"type", "ASTRoot"},
                             {"children",
                              {
                                      {{"type", "ExprStmt"},
                                       {
                                               "expr",
                                               {{"type", "Binary"},
                                                {"op", "Add"},
                                                {"lhs",
                                                 {
                                                         {"type", "ConstExpr"},
                                                         {"value", "1"},
                                                         {"expr_type", "Int"},
                                                 }},
                                                {"rhs",
                                                 {
                                                         {"type", "Binary"},
                                                         {"op", "Mul"},
                                                         {"lhs",
                                                          {
                                                                  {"type", "ConstExpr"},
                                                                  {"value", "2"},
                                                                  {"expr_type", "Int"},
                                                          }},
                                                         {"rhs",
                                                          {
                                                                  {"type", "ConstExpr"},
                                                                  {"value", "3"},
                                                                  {"expr_type", "Int"},
                                                          }},
                                                 }}},
                                       }},
                              }}};
    Common::JSON json = res->toJSON();
    EXPECT_EQ(expected, json);
}
