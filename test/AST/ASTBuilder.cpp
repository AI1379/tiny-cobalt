//
// Created by Renatus Madrigal on 02/06/2025
//

#include "AST/ASTBuilder.h"
#include <gtest/gtest.h>
#include "AST/ASTNodeDecl.h"
#include "AST/ASTRootNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"

using namespace TinyCobalt;
using namespace AST;
using namespace Builder;

using std::string_literals::operator""s;

TEST(AST, ASTBuilderTest1) {
    // clang-format off
    auto ast_json = Node<VariableDefPtr>{
        Node<SimpleTypePtr>{
            "int"s
        }(),
        "a"s
    }()->toJSON();
    // clang-format on
    auto exp = R"(
    {
        "type": "VariableDef",
        "type_node": {
            "type": "SimpleType",
            "name": "int"
        },
        "name": "a",
        "init": null
    }
    )"_json;
    EXPECT_EQ(exp, ast_json);
}

TEST(AST, ASTBuilderTest2) {
    // clang-format off
    auto ast_json = Node<ASTRootPtr> { 
        Array<StmtNodePtr> {
            Node<VariableDefPtr>{Node<SimpleTypePtr>{"int"s}(), "a"s}(),
            Node<VariableDefPtr>{Node<SimpleTypePtr>{"int"s}(), "b"s}(),
            Node<VariableDefPtr>{Node<SimpleTypePtr>{"int"s}(), "c"s}()
        }()
    }()->toJSON();
    // clang-format on
    auto exp = R"(
    {
        "type": "ASTRoot",
        "children": [
            {
                "type": "VariableDef",
                "type_node": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "a",
                "init": null
            },
            {
                "type": "VariableDef",
                "type_node": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "b",
                "init": null
            },
            {
                "type": "VariableDef",
                "type_node": {
                    "type": "SimpleType",
                    "name": "int"
                },
                "name": "c",
                "init": null
            }
        ]
    }
    )"_json;
    EXPECT_EQ(exp, ast_json);
}