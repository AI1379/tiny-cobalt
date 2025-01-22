//
// Created by Renatus Madrigal on 01/22/2025
//

#include <gtest/gtest.h>
#include "AST/AST.h"
#include "Semantic/Scope.h"

using namespace TinyCobalt::Semantic;

TEST(Scope, AddSymbolTest) {
    Scope<std::string, int> scope;
    scope.addSymbol("a", 1);
    scope.addSymbol("b", 2);
    scope.addSymbol("c", 3);
    EXPECT_EQ(scope.getSymbol("a"), 1);
    EXPECT_EQ(scope.getSymbol("b"), 2);
    EXPECT_EQ(scope.getSymbol("c"), 3);
    EXPECT_EQ(scope.getSymbol("d"), std::nullopt);
}

TEST(Scope, NestedScopeTest) {
    Scope<std::string, int> scope1;
    scope1.addSymbol("a", 1);
    scope1.addSymbol("b", 2);
    scope1.addSymbol("c", 3);
    Scope<std::string, int> scope2(&scope1);
    scope2.addSymbol("d", 4);
    scope2.addSymbol("e", 5);
    scope2.addSymbol("f", 6);
    EXPECT_EQ(scope2.getSymbol("a"), 1);
    EXPECT_EQ(scope2.getSymbol("b"), 2);
    EXPECT_EQ(scope2.getSymbol("c"), 3);
    EXPECT_EQ(scope2.getSymbol("d"), 4);
    EXPECT_EQ(scope2.getSymbol("e"), 5);
    EXPECT_EQ(scope2.getSymbol("f"), 6);
    EXPECT_EQ(scope2.getSymbol("g"), std::nullopt);
}

TEST(Scope, GetLocalSymbolTest) {
    Scope<std::string, int> scope1;
    scope1.addSymbol("a", 1);
    scope1.addSymbol("b", 2);
    scope1.addSymbol("c", 3);
    Scope<std::string, int> scope2(&scope1);
    scope2.addSymbol("d", 4);
    scope2.addSymbol("e", 5);
    scope2.addSymbol("f", 6);
    EXPECT_EQ(scope2.getLocalSymbol("a"), std::nullopt);
    EXPECT_EQ(scope2.getLocalSymbol("b"), std::nullopt);
    EXPECT_EQ(scope2.getLocalSymbol("c"), std::nullopt);
    EXPECT_EQ(scope2.getLocalSymbol("d"), 4);
    EXPECT_EQ(scope2.getLocalSymbol("e"), 5);
    EXPECT_EQ(scope2.getLocalSymbol("f"), 6);
    EXPECT_EQ(scope2.getLocalSymbol("g"), std::nullopt);
}

TEST(Scope, GetSymbolWithScopeTest) {
    Scope<std::string, int> scope1;
    scope1.addSymbol("a", 1);
    scope1.addSymbol("b", 2);
    scope1.addSymbol("c", 3);
    Scope<std::string, int> scope2(&scope1);
    scope2.addSymbol("d", 4);
    scope2.addSymbol("e", 5);
    scope2.addSymbol("f", 6);
    EXPECT_EQ(scope2.getSymbolWithScope("d")->second, &scope2);
    EXPECT_EQ(scope2.getSymbolWithScope("e")->second, &scope2);
    EXPECT_EQ(scope2.getSymbolWithScope("f")->second, &scope2);
    EXPECT_EQ(scope2.getSymbolWithScope("a")->second, &scope1);
    EXPECT_EQ(scope2.getSymbolWithScope("b")->second, &scope1);
    EXPECT_EQ(scope2.getSymbolWithScope("c")->second, &scope1);
    EXPECT_EQ(scope2.getSymbolWithScope("g"), std::nullopt);
}

TEST(Scope, GetNameTest) {
    Scope<std::string, int> scope1(nullptr, "scope1");
    Scope<std::string, int> scope2(&scope1, "scope2");
    Scope<std::string, int> scope3(&scope2, "scope3");
    Scope<std::string, int> scope4(&scope2);
    Scope<std::string, int> scope5(&scope4, "scope5");
    EXPECT_EQ(scope1.getName(), "scope1");
    EXPECT_EQ(scope2.getName(), "scope2");
    EXPECT_EQ(scope3.getName(), "scope3");
    EXPECT_EQ(scope4.getName(), "<anonymous>");
    EXPECT_EQ(scope3.getFullName(), "scope1::scope2::scope3");
    EXPECT_EQ(scope4.getFullName(), "scope1::scope2::<anonymous>");
    EXPECT_EQ(scope5.getFullName(), "scope1::scope2::<anonymous>::scope5");
}
