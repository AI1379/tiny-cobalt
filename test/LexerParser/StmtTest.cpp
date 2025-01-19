//
// Created by Renatus Madrigal on 01/19/2025
//

#include <gtest/gtest.h>
#include <sstream>
#include "AST/AST.h"
#include "Common/JSON.h"
#include "LexerParser/Parser.h"

using namespace TinyCobalt;

#define INIT_TEST                                                                                                      \
    LexerParser::Parser parser;                                                                                        \
    std::istringstream is(input);                                                                                      \
    std::ostringstream os;                                                                                             \
    parser.switchInput(&is).switchOutput(&os);                                                                         \
    auto err = parser.parse();                                                                                         \
    ASSERT_EQ(err, 0);                                                                                                 \
    auto res = parser.result();                                                                                        \
    Common::JSON json = res->toJSON();

TEST(LexerParser, IfStmt1) {
    std::string input = R"(if (a) b;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "then_stmt": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "b"
                    }
                },
                "else_stmt": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, IfStmt2) {
    std::string input = R"(if (a) b; else c;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "then_stmt": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "b"
                    }
                },
                "else_stmt": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "c"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, IfStmt3) {
    std::string input = R"(
        if (a && b || c) {
            d;
        } else {
            e;
        }
    )";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Binary",
                    "op": "Or",
                    "lhs": {
                        "type": "Binary",
                        "op": "And",
                        "lhs": {
                            "type": "Variable",
                            "name": "a"
                        },
                        "rhs": {
                            "type": "Variable",
                            "name": "b"
                        }
                    },
                    "rhs": {
                        "type": "Variable",
                        "name": "c"
                    }
                },
                "then_stmt": {
                    "type": "Block",
                    "stmts": [
                        {
                            "type": "ExprStmt",
                            "expr": {
                                "type": "Variable",
                                "name": "d"
                            }
                        }
                    ]
                },
                "else_stmt": {
                    "type": "Block",
                    "stmts": [
                        {
                            "type": "ExprStmt",
                            "expr": {
                                "type": "Variable",
                                "name": "e"
                            }
                        }
                    ]
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, IfStmt4) {
    std::string input = R"(if(a) if (b) c; else d;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "then_stmt": {
                    "type": "If",
                    "condition": {
                        "type": "Variable",
                        "name": "b"
                    },
                    "then_stmt": {
                        "type": "ExprStmt",
                        "expr": {
                            "type": "Variable",
                            "name": "c"
                        }
                    },
                    "else_stmt": {
                        "type": "ExprStmt",
                        "expr": {
                            "type": "Variable",
                            "name": "d"
                        }
                    }
                },
                "else_stmt": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, IfStmt5) {
    std::string input = R"(if(a) { if(b) c; } else d;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "then_stmt": {
                    "type": "Block",
                    "stmts": [
                        {
                            "type": "If",
                            "condition": {
                                "type": "Variable",
                                "name": "b"
                            },
                            "then_stmt": {
                                "type": "ExprStmt",
                                "expr": {
                                    "type": "Variable",
                                    "name": "c"
                                }
                            },
                            "else_stmt": null
                        }
                    ]
                },
                "else_stmt": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "d"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, IfStmt6) {
    std::string input = R"(if(a) b; else if(c) d; else e;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "If",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "then_stmt": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "b"
                    }
                },
                "else_stmt": {
                    "type": "If",
                    "condition": {
                        "type": "Variable",
                        "name": "c"
                    },
                    "then_stmt": {
                        "type": "ExprStmt",
                        "expr": {
                            "type": "Variable",
                            "name": "d"
                        }
                    },
                    "else_stmt": {
                        "type": "ExprStmt",
                        "expr": {
                            "type": "Variable",
                            "name": "e"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

// TODO: tests for syntax error

TEST(LexerParser, WhileStmt1) {
    std::string input = R"(while (a) b;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "While",
                "condition": {
                    "type": "Variable",
                    "name": "a"
                },
                "body": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "b"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

// Currently we haven't support empty block
TEST(LexerParser, ForStmt1) {
    std::string input = R"(for (a; b; c) d;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "For",
                "init": {
                    "type": "Variable",
                    "name": "a"
                },
                "condition": {
                    "type": "Variable",
                    "name": "b"
                },
                "step": {
                    "type": "Variable",
                    "name": "c"
                },
                "body": {
                    "type": "ExprStmt",
                    "expr": {
                        "type": "Variable",
                        "name": "d"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ReturnStmt1) {
    std::string input = R"(return ;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "Return",
                "value": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ReturnStmt2) {
    std::string input = R"(return a;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "Return",
                "value": {
                    "type": "Variable",
                    "name": "a"
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ReturnStmt3) {
    std::string input = R"(return a && b;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "Return",
                "value": {
                    "type": "Binary",
                    "op": "And",
                    "lhs": {
                        "type": "Variable",
                        "name": "a"
                    },
                    "rhs": {
                        "type": "Variable",
                        "name": "b"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, VariableDefStmt1) {
    std::string input = R"(int a;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "a",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, VariableDefStmt2) {
    std::string input = R"(int a = 3;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "a",
                "init": {
                    "type": "ConstExpr",
                    "value": "3",
                    "expr_type": "Int"
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, VariableDefStmt3) {
    std::string input = R"(int a = 3 + 4;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "a",
                "init": {
                    "type": "Binary",
                    "op": "Add",
                    "lhs": {
                        "type": "ConstExpr",
                        "value": "3",
                        "expr_type": "Int"
                    },
                    "rhs": {
                        "type": "ConstExpr",
                        "value": "4",
                        "expr_type": "Int"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, FuncDefStmt1) {
    std::string input = R"(int main() { return 0; })";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "FuncDef",
                "return_type": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "main",
                "params": [],
                "body": {
                    "type": "Block",
                    "stmts": [
                        {
                            "type": "Return",
                            "value": {
                                "type": "ConstExpr",
                                "value": "0",
                                "expr_type": "Int"
                            }
                        }
                    ]
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, StructDefStmt1) {
    std::string input = R"(struct A { int a; int b; };)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "StructDef",
                "name": "A",
                "fields": [
                    {
                        "type": "VariableDef",
                        "type": {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        "name": "a",
                        "init": null
                    },
                    {
                        "type": "VariableDef",
                        "type": {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        "name": "b",
                        "init": null
                    }
                ]
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, StuctDefStmt2) {
    std::string input = R"(struct A{};)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "StructDef",
                "name": "A",
                "fields": []
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, AliasDefStmt1) {
    std::string input = R"(using A = int;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "AliasDef",
                "name": "A",
                "type": {
                    "type": "SimpleType",
                    "name": "int"
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, EmptyStmt1) {
    std::string input = ";;;;;";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            { "type": "EmptyStmt" },
            { "type": "EmptyStmt" },
            { "type": "EmptyStmt" },
            { "type": "EmptyStmt" },
            { "type": "EmptyStmt" }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}
