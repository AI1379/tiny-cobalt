//
// Created by Renatus Madrigal on 12/15/2024
//

#include <gtest/gtest.h>
#include <sstream>

#include "Common/Generator.h"
#include "Common/Range.h"

using TinyCobalt::Ranges::ElementsOf;
using TinyCobalt::Utility::Generator;

template<typename T>
struct Tree {
    T value;
    Tree *left{}, *right{};
    Generator<const T &> traverse() {
        if (left) {
            co_yield ElementsOf(left->traverse());
        }
        co_yield value;
        if (right) {
            co_yield ElementsOf(right->traverse());
        }
    }
    Generator<Tree<T> &> children() {
        if (left) {
            co_yield *left;
        }
        if (right) {
            co_yield *right;
        }
    }
};

TEST(Common, Generator1) {
    // Test from https://en.cppreference.com/w/cpp/coroutine/generator
    Tree<char> tree[]{{'D', tree + 1, tree + 2},
                      {'B', tree + 3, tree + 4},
                      {'F', tree + 5, tree + 6},
                      {'A'},
                      {'C'},
                      {'E'},
                      {'G'}};
    std::stringstream ss;
    for (auto x: tree->traverse()) {
        ss << x << ' ';
    }
    EXPECT_EQ(ss.str(), "A B C D E F G ");
}

TEST(Common, Generator2) {
    // Test from https://en.cppreference.com/w/cpp/coroutine/generator
    Tree<char> tree[]{{'D', tree + 1, tree + 2},
                      {'B', tree + 3, tree + 4},
                      {'F', tree + 5, tree + 6},
                      {'A'},
                      {'C'},
                      {'E'},
                      {'G'}};
    std::stringstream ss;
    auto f = [&ss](auto &&self, Tree<char> &t) -> void {
        ss << t.value << ' ';
        for (auto &child: t.children()) {
            self(self, child);
        }
    };
    f(f, tree[0]);
    EXPECT_EQ(ss.str(), "D B A C F E G ");
}