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

TEST(LexerParser, FuncType1) {
    std::string input = R"(int(int, int) x;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "FuncType",
                    "return_type": {
                        "type": "SimpleType",
                        "name": "int"
                    },
                    "param_types": [
                        {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        {
                            "type": "SimpleType",
                            "name": "int"
                        }
                    ]
                },
                "name": "x",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, FuncType2) {
    std::string input = R"(int(int, void()) x;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "FuncType",
                    "return_type": {
                        "type": "SimpleType",
                        "name": "int"
                    },
                    "param_types": [
                        {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        {
                            "type": "FuncType",
                            "return_type": {
                                "type": "SimpleType",
                                "name": "void"
                            },
                            "param_types": []
                        }
                    ]
                },
                "name": "x",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, FuncType3) {
    std::string input = R"(void() x;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "FuncType",
                    "return_type": {
                        "type": "SimpleType",
                        "name": "void"
                    },
                    "param_types": []
                },
                "name": "x",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ComplexType1) {
    std::string input = R"(Pointer<int> a;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "ComplexType",
                    "template_name": "Pointer",
                    "template_args": [{
                        "type": "SimpleType",
                        "name": "int"
                    }]
                },
                "name": "a",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ComplexType2) {
    std::string input = R"(Map<int, Map<int, int> > mp;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "ComplexType",
                    "template_name": "Map",
                    "template_args": [
                        {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        {
                            "type": "ComplexType",
                            "template_name": "Map",
                            "template_args": [
                                {
                                    "type": "SimpleType",
                                    "name": "int"
                                },
                                {
                                    "type": "SimpleType",
                                    "name": "int"
                                }
                            ]
                        }
                    ]
                },
                "name": "mp",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ComplexType3) {
    std::string input = R"(Map<int, int(Pointer<char>)> callbacks;)";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "ComplexType",
                    "template_name": "Map",
                    "template_args": [
                        {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        {
                            "type": "FuncType",
                            "return_type": {
                                "type": "SimpleType",
                                "name": "int"
                            },
                            "param_types": [
                                {
                                    "type": "ComplexType",
                                    "template_name": "Pointer",
                                    "template_args": [
                                        {
                                            "type": "SimpleType",
                                            "name": "char"
                                        }
                                    ]
                                }
                            ]
                        }
                    ]
                },
                "name": "callbacks",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ComplexType4) {
    std::string input = "Pointer<int>() x;";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "FuncType",
                    "return_type": {
                        "type": "ComplexType",
                        "template_name": "Pointer",
                        "template_args": [
                            {
                                "type": "SimpleType",
                                "name": "int"
                            }
                        ]
                    },
                    "param_types": []
                },
                "name": "x",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}

TEST(LexerParser, ComplexType5) {
    std::string input = "Tuple<int, Pointer<int>, Map<int, int> > x;";
    INIT_TEST;
    Common::JSON expected = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type": {
                    "type": "ComplexType",
                    "template_name": "Tuple",
                    "template_args": [
                        {
                            "type": "SimpleType",
                            "name": "int"
                        },
                        {
                            "type": "ComplexType",
                            "template_name": "Pointer",
                            "template_args": [
                                {
                                    "type": "SimpleType",
                                    "name": "int"
                                }
                            ]
                        },
                        {
                            "type": "ComplexType",
                            "template_name": "Map",
                            "template_args": [
                                {
                                    "type": "SimpleType",
                                    "name": "int"
                                },
                                {
                                    "type": "SimpleType",
                                    "name": "int"
                                }
                            ]
                        }
                    ]
                },
                "name": "x",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(expected, json);
}