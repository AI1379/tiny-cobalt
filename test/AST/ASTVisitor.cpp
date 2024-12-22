//
// Created by Renatus Madrigal on 12/22/2024
//

#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <optional>
#include <proxy.h>
#include <sstream>

#include "AST/ASTNode.h"
#include "AST/ASTVisitor.h"
#include "Common/Dumper.h"


using namespace TinyCobalt;

// We define several TestNode type that fits all ASTNode Concepts.
struct BinaryNode : public AST::EnableThisPointer<BinaryNode> {
    AST::ASTNodePtr left, right;
    template<typename L, typename R>
        requires AST::ASTNodePtrConcept<L> && AST::ASTNodePtrConcept<R>
    BinaryNode(L left, R right) : left(left), right(right) {}

    AST::ASTNodeGen traverse() {
        co_yield left;
        co_yield right;
    }
};

struct UnaryNode : public AST::EnableThisPointer<UnaryNode> {
    AST::ASTNodePtr child;

    template<typename C>
        requires AST::ASTNodePtrConcept<C>
    UnaryNode(C child) : child(child) {}

    AST::ASTNodeGen traverse() { co_yield child; }
};

struct LeafNode : public AST::EnableThisPointer<LeafNode> {
    explicit LeafNode(std::string name) : name(std::move(name)) {}
    LeafNode(const LeafNode &) = default;

    // TODO: co_return
    AST::ASTNodeGen traverse() { co_return; }
    std::string name;
    bool visited = false;
};

static_assert(AST::ASTNodeConcept<BinaryNode>, "BinaryNode should satisfy ASTNodeConcept");
static_assert(AST::ASTNodeConcept<UnaryNode>, "UnaryNode should satisfy ASTNodeConcept");
static_assert(AST::ASTNodeConcept<LeafNode>, "LeafNode should satisfy ASTNodeConcept");

// We define a visitor that dump these nodes
class DumperVisitor : public AST::BaseASTVisitor<DumperVisitor> {
public:
    explicit DumperVisitor(Utility::Dumper &dumper) : dumper_(dumper) {}

    void beforeSubtree(AST::ASTNodePtr node) {
        if (node->containType<BinaryNode>()) {
            dumper_.endl().dump("BinaryNode").startBlock();
        } else if (node->containType<UnaryNode>()) {
            dumper_.endl().dump("UnaryNode").startBlock();
        } else if (node->containType<LeafNode>()) {
            dumper_.endl().dump(node->cast<LeafNode>()->name);
            node->cast<LeafNode>()->visited = true;
        }
    }

    void afterSubtree(AST::ASTNodePtr node) {
        if (node->containType<BinaryNode>()) {
            dumper_.endBlock().endl().dump("EndBinaryNode");
        } else if (node->containType<UnaryNode>()) {
            dumper_.endBlock().endl().dump("EndUnaryNode");
        }
    }

private:
    Utility::Dumper &dumper_;
};

static_assert(AST::ASTVisitorConcept<DumperVisitor>, "DumperVisitor should satisfy ASTVisitorConcept");

TEST(AST, ASTVisitor) {
    const std::string expected = "BinaryNode{BinaryNode{leaf1leaf2}EndBinaryNodeUnaryNode{BinaryNode{leaf3leaf4}"
                                 "EndBinaryNode}EndUnaryNode}EndBinaryNode";
    std::stringstream ss;
    Utility::Dumper dumper(ss);
    dumper.setEndLine("");
    DumperVisitor visitor(dumper);
    auto leaf1 = std::make_shared<LeafNode>("leaf1");
    auto leaf2 = std::make_shared<LeafNode>("leaf2");
    auto leaf3 = std::make_shared<LeafNode>("leaf3");
    auto leaf4 = std::make_shared<LeafNode>("leaf4");
    auto binary1 = std::make_shared<BinaryNode>(leaf1, leaf2);
    auto binary2 = std::make_shared<BinaryNode>(leaf3, leaf4);
    auto unary = std::make_shared<UnaryNode>(binary2);
    auto root = std::make_shared<BinaryNode>(binary1, unary);

    AST::ASTNodePtr rootPtr = root;
    visitor.visit(rootPtr);
    EXPECT_TRUE(leaf1->visited);
    EXPECT_TRUE(leaf2->visited);
    EXPECT_TRUE(leaf3->visited);
    EXPECT_TRUE(leaf4->visited);
    EXPECT_EQ(ss.str(), expected);
}
