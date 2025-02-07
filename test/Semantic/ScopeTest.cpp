//
// Created by Renatus Madrigal on 01/22/2025
//

#include <gtest/gtest.h>
#include <memory>
#include "AST/AST.h"
#include "Semantic/Scope.h"

using namespace TinyCobalt::Semantic;

using ScopeSI = Scope<std::string, int>;

TEST(Scope, AddSymbolTest) {
    ScopeSI scope;
    scope.addSymbol("a", 1);
    scope.addSymbol("b", 2);
    scope.addSymbol("c", 3);
    EXPECT_EQ(scope.getSymbol("a"), 1);
    EXPECT_EQ(scope.getSymbol("b"), 2);
    EXPECT_EQ(scope.getSymbol("c"), 3);
    EXPECT_EQ(scope.getSymbol("d"), std::nullopt);
}

TEST(Scope, NestedScopeTest) {
    auto scope1 = std::make_unique<ScopeSI>();
    scope1->addSymbol("a", 1);
    scope1->addSymbol("b", 2);
    scope1->addSymbol("c", 3);
    auto scope2 = std::make_unique<ScopeSI>(std::move(scope1));
    EXPECT_EQ(scope1, nullptr);
    scope2->addSymbol("d", 4);
    scope2->addSymbol("e", 5);
    scope2->addSymbol("f", 6);
    EXPECT_EQ(scope2->getSymbol("a"), 1);
    EXPECT_EQ(scope2->getSymbol("b"), 2);
    EXPECT_EQ(scope2->getSymbol("c"), 3);
    EXPECT_EQ(scope2->getSymbol("d"), 4);
    EXPECT_EQ(scope2->getSymbol("e"), 5);
    EXPECT_EQ(scope2->getSymbol("f"), 6);
    EXPECT_EQ(scope2->getSymbol("g"), std::nullopt);
}

TEST(Scope, GetLocalSymbolTest) {
    auto scope1 = std::make_unique<ScopeSI>();
    scope1->addSymbol("a", 1);
    scope1->addSymbol("b", 2);
    scope1->addSymbol("c", 3);
    auto scope2 = std::make_unique<ScopeSI>(std::move(scope1));
    EXPECT_EQ(scope1, nullptr);
    scope2->addSymbol("d", 4);
    scope2->addSymbol("e", 5);
    scope2->addSymbol("f", 6);
    EXPECT_EQ(scope2->getLocalSymbol("a"), std::nullopt);
    EXPECT_EQ(scope2->getLocalSymbol("b"), std::nullopt);
    EXPECT_EQ(scope2->getLocalSymbol("c"), std::nullopt);
    EXPECT_EQ(scope2->getLocalSymbol("d"), 4);
    EXPECT_EQ(scope2->getLocalSymbol("e"), 5);
    EXPECT_EQ(scope2->getLocalSymbol("f"), 6);
    EXPECT_EQ(scope2->getLocalSymbol("g"), std::nullopt);
}

TEST(Scope, GetSymbolWithScopeTest) {
    auto scope1 = std::make_unique<ScopeSI>();
    auto scope1ptr = scope1.get();
    scope1->addSymbol("a", 1);
    scope1->addSymbol("b", 2);
    scope1->addSymbol("c", 3);
    auto scope2 = std::make_unique<ScopeSI>(std::move(scope1));
    auto scope2ptr = scope2.get();
    EXPECT_EQ(scope1, nullptr);
    EXPECT_EQ(scope2->getObserverParent(), scope1ptr);
    scope2->addSymbol("d", 4);
    scope2->addSymbol("e", 5);
    scope2->addSymbol("f", 6);
    EXPECT_EQ(scope2->getSymbolWithScope("d")->second, scope2ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("e")->second, scope2ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("f")->second, scope2ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("a")->second, scope1ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("b")->second, scope1ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("c")->second, scope1ptr);
    EXPECT_EQ(scope2->getSymbolWithScope("g"), std::nullopt);
}

// TODO: Because the parent_ pointer in scope is held by a unique_ptr, it is not possible to construct a tree of scopes.
#if 0
TEST(Scope, GetNameTest) {
    auto scope1 = std::make_unique<ScopeSI>(nullptr, "scope1");
    auto scope1ptr = scope1.get();
    auto scope2 = std::make_unique<ScopeSI>(std::move(scope1), "scope2");
    auto scope2ptr = scope2.get();
    auto scope3 = std::make_unique<ScopeSI>(std::move(scope2), "scope3");
    auto scope3ptr = scope3.get();
    auto scope4 = std::make_unique<ScopeSI>(std::move(scope2));
    auto scope4ptr = scope4.get();
    auto scope5 = std::make_unique<ScopeSI>(std::move(scope4), "scope5");
    auto scope5ptr = scope5.get();
    EXPECT_EQ(scope1ptr->getName(), "scope1");
    EXPECT_EQ(scope2ptr->getName(), "scope2");
    EXPECT_EQ(scope3ptr->getName(), "scope3");
    EXPECT_EQ(scope4ptr->getName(), "<anonymous>");
    EXPECT_EQ(scope3ptr->getFullName(), "scope1::scope2::scope3");
    EXPECT_EQ(scope4ptr->getFullName(), "scope1::scope2::<anonymous>");
    EXPECT_EQ(scope5ptr->getFullName(), "scope1::scope2::<anonymous>::scope5");
}
#endif
