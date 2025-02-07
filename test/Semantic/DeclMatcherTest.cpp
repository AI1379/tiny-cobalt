//
// Created by Renatus Madrigal on 02/06/2025
//

#include <gtest/gtest.h>
#include <memory>
#include "AST/ASTBuilder.h"
#include "AST/ASTNodeDecl.h"
#include "AST/ASTRootNode.h"
#include "AST/ASTVisitor.h"
#include "AST/ExprNode.h"
#include "AST/StmtNode.h"
#include "AST/TypeNode.h"
#include "LexerParser/Parser.h"
#include "Semantic/DeclMatcher.h"

using namespace TinyCobalt;

using DeclMatcherVisitor = AST::BaseASTVisitor<Semantic::DeclMatcher>;

using std::string_literals::operator""s;

using namespace AST;
using namespace AST::Builder;

TEST(Semantic, DeclMatcherTest1) {
    VariableDefPtr aptr, bptr, cptr;
    VariablePtr a, b, c;
    // clang-format off
    auto ast = Node<ASTRootPtr> {
        Array<StmtNodePtr> {
            aptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "a"s }(),
            bptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "b"s }(),
            cptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "c"s }(),
            Node<ExprStmtPtr> {
                Node<BinaryPtr> {
                    a = Node<VariablePtr> { "a"s }(),
                    BinaryOp::Assign,
                    Node<BinaryPtr> {
                        b = Node<VariablePtr> { "b"s }(),
                        BinaryOp::Add,
                        c = Node<VariablePtr> { "c"s }()
                    }()
                }()
            }()
        }()
    }();
    // clang-format on
    DeclMatcherVisitor visitor;
    visitor.visit(ast);
    EXPECT_EQ(a->def, aptr);
    EXPECT_EQ(b->def, bptr);
    EXPECT_EQ(c->def, cptr);
}

TEST(Semantic, DeclMatcherTest2) {
    VariableDefPtr aptr, bptr, cptr;
    VariablePtr a, b, c;
    BlockPtr block;
    FuncDefPtr func;
    // clang-format off
    auto ast = Node<ASTRootPtr> {
        Array<StmtNodePtr> {
            aptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "a"s }(),
            block = Node<BlockPtr> {
                Array<StmtNodePtr> {
                    bptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "b"s }(),
                    Node<ExprStmtPtr>{
                        Node<BinaryPtr> {
                            a = Node<VariablePtr> { "a"s }(),
                            BinaryOp::Assign,
                            Node<ConstExprPtr> {
                                "1"s,
                                ConstExprType::Int
                            }(),
                        }(),
                    }(),
                    func = Node<FuncDefPtr> {
                        Node<SimpleTypePtr> { "void"s }(),
                        "Function",
                        Array<FuncDefNode::ParamsElem>{}(),
                        Node<BlockPtr> {
                            Array<StmtNodePtr> {
                                cptr = Node<VariableDefPtr> { Node<SimpleTypePtr>{ "int"s }(), "c"s }(),
                                Node<ExprStmtPtr>{
                                    Node<BinaryPtr> {
                                        b = Node<VariablePtr> { "b"s }(),
                                        BinaryOp::Assign,
                                        Node<ConstExprPtr> {
                                            "2"s,
                                            ConstExprType::Int
                                        }()
                                    }(),
                                }()
                            }(),
                        }(),
                    }(),
                    Node<ExprStmtPtr>{
                        Node<BinaryPtr> {
                            c = Node<VariablePtr> { "c"s }(),
                            BinaryOp::Assign,
                            Node<ConstExprPtr> {
                                "3"s,
                                ConstExprType::Int
                            }(),
                        }(),
                    }(),
                }(),
            }(),
        }(),
    }();
    // clang-format on
    DeclMatcherVisitor visitor;
    visitor.visit(ast);
    EXPECT_EQ(a->def, aptr);
    EXPECT_EQ(b->def, bptr);
    EXPECT_EQ(c->def, nullptr);
}