//
// Created by Renatus Madrigal on 01/20/2025
//

#include <algorithm>
#include <concepts>
#include <functional>
#include <gtest/gtest.h>
#include <ranges>
#include <stdexcept>
#include <type_traits>
#include "Common/FlatMap.h"

using namespace TinyCobalt;
using Common::flat_map;

template<typename T1, typename T2, typename U1, typename U2>
    requires std::same_as<std::remove_cvref_t<T1>, std::remove_cvref_t<U1>> &&
             std::same_as<std::remove_cvref_t<T2>, std::remove_cvref_t<U2>>
bool operator==(std::pair<T1, T2> lhs, std::pair<U1, U2> rhs) {
    return lhs.first == rhs.first && lhs.second == rhs.second;
}

TEST(FlatMap, InsertTest1) {
    flat_map<int, int> map;
    map.insert({1, 3});
    map.insert({2, 4});
    map.insert({3, 5});
    EXPECT_EQ(map.size(), 3);
    EXPECT_EQ(map[1], 3);
    EXPECT_EQ(map[2], 4);
    EXPECT_EQ(map[3], 5);
    EXPECT_THROW(map.at(5), std::out_of_range);
    EXPECT_EQ(map[5], 0);
    map[5] = 7;
    EXPECT_EQ(map[5], 7);
}

TEST(FlatMap, InsertTest2) {
    flat_map<int, int> map;
    map.insert({1, -1});
    map.insert({2, -2});
    map.insert({3, -3});
    map.insert({1, -4});
    map.insert({2, -5});
    map.insert({3, -6});
    map.insert({0, 0});
    EXPECT_EQ(map.size(), 4);
    EXPECT_TRUE(std::ranges::equal(map.keys(), (int[]) {0, 1, 2, 3}));
    EXPECT_TRUE(std::ranges::equal(map.values(), (int[]) {0, -1, -2, -3}));
}

TEST(FlatMap, InsertTest3) {
    flat_map<int, int> map;
    for (int i = 0; i < 4; ++i) {
        map.clear();
        int j = i;
        map.insert(map.end(), {j, -j});
        j = (j + 1) % 4;
        map.insert(map.end(), {j, -j});
        j = (j + 1) % 4;
        map.insert(map.end(), {j, -j});
        j = (j + 1) % 4;
        map.insert(map.end(), {j, -j});

        map.insert(map.begin() + i, {1, -4});
        map.insert(map.begin() + i, {2, -5});
        map.insert(map.begin() + i, {3, -6});
        map.insert(map.begin() + i, {0, -7});
        EXPECT_TRUE(std::ranges::equal(map.keys(), (int[]) {0, 1, 2, 3}));
        EXPECT_TRUE(std::ranges::equal(map.values(), (int[]) {0, -1, -2, -3}));
    }
}

TEST(FlatMap, InsertTest4) {
    flat_map<int, int, std::greater<int>> map;
    static_assert(std::ranges::random_access_range<decltype(map)>);
    auto r = map.insert({1, -1});
    EXPECT_TRUE(r.first->first == 1 && r.first->second == -1 && r.second);
    r = map.insert({2, -2});
    EXPECT_TRUE(r.first->first == 2 && r.first->second == -2 && r.second);
    r = map.insert({3, -3});
    EXPECT_TRUE(r.first->first == 3 && r.first->second == -3 && r.second);
    r = map.insert({1, -4});
    EXPECT_TRUE(r.first->first == 1 && r.first->second == -1 && !r.second);
    r = map.insert({2, -5});
    EXPECT_TRUE(r.first->first == 2 && r.first->second == -2 && !r.second);
    r = map.insert({3, -6});
    EXPECT_TRUE(r.first->first == 3 && r.first->second == -3 && !r.second);
    r = map.insert_or_assign(0, 0);
    EXPECT_TRUE(r.first->first == 0 && r.first->second == 0 && r.second);
    r = map.insert_or_assign(0, 1);
    EXPECT_TRUE(r.first->first == 0 && r.first->second == 1 && !r.second);
    EXPECT_TRUE(*map.insert_or_assign(map.end(), 0, 2) == std::pair(0, 2));
    EXPECT_TRUE(map.size() == 4);
    EXPECT_TRUE(std::ranges::equal(map.keys(), (int[]) {3, 2, 1, 0}));
    EXPECT_TRUE(std::ranges::equal(map.values(), (int[]) {-3, -2, -1, 2}));
    EXPECT_TRUE(map.contains(3) && !map.contains(7));
    EXPECT_TRUE(map.count(3) == 1);
}

TEST(FlatMap, EraseTest1) {
    flat_map<int, int> map;
    map = {std::pair(1, 2), {3, 4}, {5, 6}};
    map.insert({std::pair(7, 8), {9, 10}});

    auto it = map.find(0);
    EXPECT_TRUE(it == map.end());
    it = map.find(9);
    EXPECT_TRUE(it->second == 10);

    const auto map2 = map;
    EXPECT_TRUE(map == map);
    EXPECT_TRUE(map == map2);

    map.erase(map.begin());
    map.erase(5);
    map.erase(map.end() - 2, map.end());
    EXPECT_TRUE(std::ranges::equal(map, (std::pair<int, int>[]) {{3, 4}}, [](auto lhs, auto rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }));
    EXPECT_TRUE(map != map2);
    EXPECT_TRUE(map2 < map);

    map = map2;
    erase_if(map, [](const auto &x) {
        auto [k, v] = x;
        return k < 5 || k > 5;
    });
    EXPECT_TRUE(std::ranges::equal(map, (std::pair<int, int>[]) {{5, 6}}, [](auto lhs, auto rhs) {
        return lhs.first == rhs.first && lhs.second == rhs.second;
    }));
}