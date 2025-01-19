//
// Created by Renatus Madrigal on 01/18/2025
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

TEST(LexerParser, BinaryExpr1) {
    std::string input = R"(1 + 2 * 3;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Add",
                    "lhs": {
                        "type": "ConstExpr",
                        "value": "1",
                        "expr_type": "Int"
                    },
                    "rhs": {
                        "type": "Binary",
                        "op": "Mul",
                        "lhs": {
                            "type": "ConstExpr",
                            "value": "2",
                            "expr_type": "Int"
                        },
                        "rhs": {
                            "type": "ConstExpr",
                            "value": "3",
                            "expr_type": "Int"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, BinaryExpr2) {
    std::string input = R"((1 + '2') * 3;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Mul",
                    "lhs": {
                        "type": "Binary",
                        "op": "Add",
                        "lhs": {
                            "type": "ConstExpr",
                            "value": "1",
                            "expr_type": "Int"
                        },
                        "rhs": {
                            "type": "ConstExpr",
                            "value": "'2'",
                            "expr_type": "Char"
                        }
                    },
                    "rhs": {
                        "type": "ConstExpr",
                        "value": "3",
                        "expr_type": "Int"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, BinaryExpr3) {
    std::string input = R"(x = (a + (b += 3));)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Assign",
                    "lhs": {
                        "type": "Variable",
                        "name": "x"
                    },
                    "rhs": {
                        "type": "Binary",
                        "op": "Add",
                        "lhs": {
                            "type": "Variable",
                            "name": "a"
                        },
                        "rhs": {
                            "type": "Binary",
                            "op": "AddAssign",
                            "lhs": {
                                "type": "Variable",
                                "name": "b"
                            },
                            "rhs": {
                                "type": "ConstExpr",
                                "value": "3",
                                "expr_type": "Int"
                            }
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, BinaryExpr4) {
    std::string input = R"(1 << b > c;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Greater",
                    "lhs": {
                        "type": "Binary",
                        "op": "BitLShift",
                        "lhs": {
                            "type": "ConstExpr",
                            "value": "1",
                            "expr_type": "Int"
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
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, BinaryExpr5) {
    std::string input = R"(1 << (b > c);)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "BitLShift",
                    "lhs": {
                        "type": "ConstExpr",
                        "value": "1",
                        "expr_type": "Int"
                    },
                    "rhs": {
                        "type": "Binary",
                        "op": "Greater",
                        "lhs": {
                            "type": "Variable",
                            "name": "b"
                        },
                        "rhs": {
                            "type": "Variable",
                            "name": "c"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, BinaryExpr6) {
    std::string input = R"(a.mem = b->exp + *c - &d;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Assign",
                    "lhs": {
                        "type": "Binary",
                        "lhs": {
                            "type": "Variable",
                            "name": "a"
                        },
                        "op": "Member",
                        "rhs": {
                            "type": "Variable",
                            "name": "mem"
                        }
                    },
                    "rhs": {
                        "type": "Binary",
                        "op": "Sub",
                        "lhs": {
                            "type": "Binary",
                            "op": "Add",
                            "lhs": {
                                "type": "Binary",
                                "lhs": {
                                    "type": "Variable",
                                    "name": "b"
                                },
                                "op": "PtrMember",
                                "rhs": {
                                    "type": "Variable",
                                    "name": "exp"
                                }
                            },
                            "rhs": {
                                "type": "Unary",
                                "op": "Deref",
                                "operand": {
                                    "type": "Variable",
                                    "name": "c"
                                }
                            }
                        },
                        "rhs": {
                            "type": "Unary",
                            "op": "Addr",
                            "operand": {
                                "type": "Variable",
                                "name": "d"
                            }
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr1) {
    std::string input = R"(-1 + 2;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Add",
                    "lhs": {
                        "type": "Unary",
                        "op": "Negative",
                        "operand": {
                            "type": "ConstExpr",
                            "value": "1",
                            "expr_type": "Int"
                        }
                    },
                    "rhs": {
                        "type": "ConstExpr",
                        "value": "2",
                        "expr_type": "Int"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr2) {
    std::string input = R"(!true;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Unary",
                    "op": "Not",
                    "operand": {
                        "type": "ConstExpr",
                        "value": "true",
                        "expr_type": "Bool"
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr3) {
    std::string input = R"(b = a+++1;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Assign",
                    "lhs": {
                        "type": "Variable",
                        "name": "b"
                    },
                    "rhs": {
                        "type": "Binary",
                        "op": "Add",
                        "lhs": {
                            "type": "Unary",
                            "op": "PostInc",
                            "operand": {
                                "type": "Variable",
                                "name": "a"
                            }
                        },
                        "rhs": {
                            "type": "ConstExpr",
                            "value": "1",
                            "expr_type": "Int"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr4) {
    std::string input = R"(b = a++--;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Binary",
                    "op": "Assign",
                    "lhs": {
                        "type": "Variable",
                        "name": "b"
                    },
                    "rhs": {
                        "type": "Unary",
                        "op": "PostDec",
                        "operand": {
                            "type": "Unary",
                            "op": "PostInc",
                            "operand": {
                                "type": "Variable",
                                "name": "a"
                            }
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr5) {
    std::string input = R"(*p++;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Unary",
                    "op": "Deref",
                    "operand": {
                        "type": "Unary",
                        "op": "PostInc",
                        "operand": {
                            "type": "Variable",
                            "name": "p"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr6) {
    std::string input = R"(++*p;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Unary",
                    "op": "PreInc",
                    "operand": {
                        "type": "Unary",
                        "op": "Deref",
                        "operand": {
                            "type": "Variable",
                            "name": "p"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, UnaryExpr7) {
    std::string input = R"(++(*p);)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "ExprStmt",
                "expr": {
                    "type": "Unary",
                    "op": "PreInc",
                    "operand": {
                        "type": "Unary",
                        "op": "Deref",
                        "operand": {
                            "type": "Variable",
                            "name": "p"
                        }
                    }
                }
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, MultiaryExpr1) {
    std::string input = "a = fun(1, 2, v[3]);";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "children": [
            {
                "expr": {
                    "lhs": {
                        "name": "a",
                        "type": "Variable"
                    },
                    "op": "Assign",
                    "rhs": {
                        "object": "fun",
                        "op": "FuncCall",
                        "operands": [
                            {
                                "expr_type": "Int",
                                "type": "ConstExpr",
                                "value": "1"
                            },
                            {
                                "expr_type": "Int",
                                "type": "ConstExpr",
                                "value": "2"
                            },
                            {
                                "object": "v",
                                "op": "Subscript",
                                "operands": [
                                    {
                                        "expr_type": "Int",
                                        "type": "ConstExpr",
                                        "value": "3"
                                    }
                                ],
                                "type": "Multiary"
                            }
                        ],
                        "type": "Multiary"
                    },
                    "type": "Binary"
                },
                "type": "ExprStmt"
            }
        ],
        "type": "ASTRoot"
    }
    )"_json;
    EXPECT_EQ(expected, json);
}
