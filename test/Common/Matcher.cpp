//
// Created by Renatus Madrigal on 02/04/2025
//

#include <gtest/gtest.h>
#include <memory>
#include <proxy.h>
#include <string>
#include "Common/Utility.h"

using namespace TinyCobalt;

struct RttiAware // NOLINT
    : pro::facade_builder // NOLINT
      ::support_indirect_rtti // NOLINT
      ::support_direct_rtti // NOLINT
      ::build {};

using RttiProxy = pro::proxy<RttiAware>;

enum class Type {
    None,
    Int,
    IntRawPtr,
    IntSharedPtr,
    IntUniquePtr,
    Double,
    DoubleRawPtr,
    DoubleSharedPtr,
    DoubleUniquePtr,
    String,
    StringRawPtr,
    StringSharedPtr,
    StringUniquePtr,
};

TEST(Common, MatcherVoidTest1) {
    int i = 114;
    double d = 5.14;
    std::string s = "Elysia";
    Type type = Type::None;
    auto match = Matcher{[&](int *) { type = Type::Int; }, // NOLINT
                         [&](double *) { type = Type::Double; }, // NOLINT
                         [&](std::string *) { type = Type::String; }};
    RttiProxy ptr = &i;
    visit(match, ptr);
    EXPECT_EQ(type, Type::Int);
    ptr = &d;
    visit(match, ptr);
    EXPECT_EQ(type, Type::Double);
    ptr = &s;
    visit(match, ptr);
    EXPECT_EQ(type, Type::String);
}

TEST(Common, MatcherVoidTest2) {
    int i = 114;
    double d = 5.14;
    std::string s = "Elysia";
    Type type = Type::None;
    auto match = Matcher{
            [&](int *) { type = Type::IntRawPtr; }, // NOLINT
            [&](double *) { type = Type::DoubleRawPtr; }, // NOLINT
            [&](std::string *) { type = Type::StringRawPtr; }, // NOLINT
            [&](std::shared_ptr<int>) { type = Type::IntSharedPtr; }, // NOLINT
            [&](std::shared_ptr<double>) { type = Type::DoubleSharedPtr; }, // NOLINT
            [&](std::shared_ptr<std::string>) { type = Type::StringSharedPtr; }, // NOLINT
            [&](std::unique_ptr<int>) { type = Type::IntUniquePtr; }, // NOLINT
            [&](std::unique_ptr<double>) { type = Type::DoubleUniquePtr; }, // NOLINT
            [&](std::unique_ptr<std::string>) { type = Type::StringUniquePtr; }, // NOLINT
    };
    RttiProxy ptr = &i;
    visit(match, ptr);
    EXPECT_EQ(type, Type::IntRawPtr);
    ptr = &d;
    visit(match, ptr);
    EXPECT_EQ(type, Type::DoubleRawPtr);
    ptr = &s;
    visit(match, ptr);
    EXPECT_EQ(type, Type::StringRawPtr);
    ptr = std::make_shared<int>(i);
    visit(match, ptr);
    EXPECT_EQ(type, Type::IntSharedPtr);
    ptr = std::make_shared<double>(d);
    visit(match, ptr);
    EXPECT_EQ(type, Type::DoubleSharedPtr);
    ptr = std::make_shared<std::string>(s);
    visit(match, ptr);
    EXPECT_EQ(type, Type::StringSharedPtr);
}

TEST(Common, MatcherNotVoidTest1) {
    int i = 114;
    double d = 5.14;
    std::string s = "Elysia";
    Type type = Type::None;
    auto match = Matcher{[](int *) -> Type { return Type::Int; }, // NOLINT
                         [](double *) -> Type { return Type::Double; }, // NOLINT
                         [](std::string *) -> Type { return Type::String; }};
    RttiProxy ptr = &i;
    type = visit(match, ptr);
    EXPECT_EQ(type, Type::Int);
    ptr = &d;
    type = visit(match, ptr);
    EXPECT_EQ(type, Type::Double);
    ptr = &s;
    type = visit(match, ptr);
    EXPECT_EQ(type, Type::String);
}

TEST(Common, MatcherNotVoidTest2) {
    int i = 114;
    double d = 5.14;
    std::string s = "Elysia";
    auto match = Matcher{[](int *i) { return std::format("{}", *i); }, // NOLINT
                         [](double *d) { return std::format("{}", *d); }, // NOLINT
                         [](std::string *s) { return std::format("{}", *s); }}; // NOLINT
    RttiProxy ptr = &i;
    EXPECT_EQ(visit(match, ptr), "114");
    ptr = &d;
    EXPECT_EQ(visit(match, ptr), "5.14");
    ptr = &s;
    EXPECT_EQ(visit(match, ptr), "Elysia");
}

TEST(Common, MatcherConstTest1) {
    std::string s = "Elysia";
    const std::string cs = "Const Elysia";
    auto match = Matcher{[](std::string *s) { return *s; }, // NOLINT
                         [](const std::string *s) { return *s; }}; // NOLINT
    RttiProxy ptr = &s;
    EXPECT_EQ(visit(match, ptr), s);
    ptr = &cs;
    EXPECT_EQ(visit(match, ptr), cs);
}

TEST(Common, MatcherFunctorTest1) {
    struct Functor {
        using Result = std::string;
        using CandidateArgs = std::tuple<int *, double *, std::string *>;
        std::string operator()(int *i) const { return std::format("{}", *i); }
        std::string operator()(double *d) const { return std::format("{}", *d); }
        std::string operator()(const std::string *s) const { return *s; }
    };
    int i = 114;
    double d = 5.14;
    std::string s = "Elysia";
    RttiProxy ptr = &i;
    EXPECT_EQ(visit(Functor{}, ptr), "114");
    ptr = &d;
    EXPECT_EQ(visit(Functor{}, ptr), "5.14");
    ptr = &s;
    EXPECT_EQ(visit(Functor{}, ptr), "Elysia");
}
