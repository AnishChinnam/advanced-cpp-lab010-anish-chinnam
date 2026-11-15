#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "project/hash_table.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

TEST_CASE("HashTable starts empty and has valid capacity") {
    HashTable table(17);
    REQUIRE(table.empty());
    REQUIRE(table.size() == 0u);
    REQUIRE(table.capacity() == 17u);
    REQUIRE(table.loadFactor() == Approx(0.0));
}

TEST_CASE("HashTable rejects zero capacity") {
    REQUIRE_THROWS_AS(HashTable(0), std::invalid_argument);
}

TEST_CASE("HashTable insert and contains basic keys") {
    HashTable table(17);
    REQUIRE(table.insert("apple", 10));
    REQUIRE(table.insert("banana", 20));
    REQUIRE(table.insert("carrot", 30));

    REQUIRE(table.size() == 3u);
    REQUIRE(table.contains("apple"));
    REQUIRE(table.contains("banana"));
    REQUIRE(table.contains("carrot"));
    REQUIRE_FALSE(table.contains("pear"));
}

TEST_CASE("HashTable rejects duplicate insertions") {
    HashTable table(11);
    REQUIRE(table.insert("alpha", 1));
    REQUIRE_FALSE(table.insert("alpha", 2));
    REQUIRE(table.size() == 1u);
    REQUIRE(table.get("alpha") == 1);
}

TEST_CASE("HashTable get and update work for existing records") {
    HashTable table(19);
    table.insert("one", 1);
    table.insert("two", 2);
    table.insert("three", 3);

    REQUIRE(table.get("one") == 1);
    REQUIRE(table.get("three") == 3);
    REQUIRE(table.update("two", 22));
    REQUIRE(table.get("two") == 22);
    REQUIRE_FALSE(table.update("missing", 99));
}

TEST_CASE("HashTable remove deletes entries and updates size") {
    HashTable table(23);
    table.insert("a", 1);
    table.insert("b", 2);
    table.insert("c", 3);

    REQUIRE(table.remove("b"));
    REQUIRE_FALSE(table.contains("b"));
    REQUIRE(table.size() == 2u);
    REQUIRE_FALSE(table.remove("missing"));
}

TEST_CASE("HashTable handles collisions with quadratic probing") {
    HashTable table(5);
    REQUIRE(table.insert("abc", 1));
    REQUIRE(table.insert("bac", 2));
    REQUIRE(table.insert("cab", 3));

    REQUIRE(table.contains("abc"));
    REQUIRE(table.contains("bac"));
    REQUIRE(table.contains("cab"));
    REQUIRE(table.get("cab") == 3);
}

TEST_CASE("HashTable rehashes when load factor grows") {
    HashTable table(5);
    for (int i = 0; i < 5; ++i) {
        table.insert("key" + std::to_string(i), i);
    }

    REQUIRE(table.capacity() > 5u);
    REQUIRE(table.size() == 5u);
    REQUIRE(table.contains("key0"));
    REQUIRE(table.contains("key4"));
}

TEST_CASE("HashTable keys and values reflect stored entries") {
    HashTable table(13);
    table.insert("alpha", 10);
    table.insert("beta", 20);
    table.insert("gamma", 30);

    auto keys = table.keys();
    auto values = table.values();

    REQUIRE(keys.size() == 3u);
    REQUIRE(values.size() == 3u);
    REQUIRE(std::find(keys.begin(), keys.end(), "alpha") != keys.end());
    REQUIRE(std::find(values.begin(), values.end(), 20) != values.end());
}

TEST_CASE("HashTable handles many strings and negative values") {
    HashTable table(31);
    table.insert("zero", 0);
    table.insert("minus-five", -5);
    table.insert("big", 123456);
    table.insert("another", -999);

    REQUIRE(table.get("zero") == 0);
    REQUIRE(table.get("minus-five") == -5);
    REQUIRE(table.get("big") == 123456);
    REQUIRE(table.get("another") == -999);
}

TEST_CASE("HashTable tombstones preserve real collision chains") {
    HashTable table(17);
    std::vector<std::string> colliding;
    for (int i = 0; colliding.size() < 4; ++i) {
        std::string key = "collision" + std::to_string(i);
        std::size_t hash = 2166136261u;
        for (unsigned char ch : key) { hash ^= ch; hash *= 16777619u; }
        if (hash % 17 == 0) colliding.push_back(key);
    }
    for (int i = 0; i < 3; ++i) REQUIRE(table.insert(colliding[i], i));
    REQUIRE(table.remove(colliding[0]));
    REQUIRE(table.get(colliding[2]) == 2);
    REQUIRE_FALSE(table.insert(colliding[2], 99));
    REQUIRE(table.insert(colliding[3], 3));
    REQUIRE(table.get(colliding[3]) == 3);
    REQUIRE(table.size() == 3u);
    REQUIRE_FALSE(table.remove(colliding[0]));
    REQUIRE_THROWS_AS(table.get(colliding[0]), std::out_of_range);
}

TEST_CASE("HashTable small capacities and deletion cycles match a reference") {
    for (std::size_t capacity : {1u, 2u, 4u, 8u}) {
        HashTable table(capacity);
        std::map<std::string, int> expected;
        REQUIRE(table.insert("", -1));
        expected[""] = -1;
        for (int i = 0; i < 400; ++i) {
            const auto key = "key" + std::to_string((i * 37) % 53);
            switch (i % 3) {
                case 0: REQUIRE(table.insert(key, i) == expected.emplace(key, i).second); break;
                case 1: REQUIRE(table.remove(key) == (expected.erase(key) != 0)); break;
                default:
                    REQUIRE(table.update(key, -i) == (expected.count(key) != 0));
                    if (expected.count(key)) expected[key] = -i;
            }
            REQUIRE(table.size() == expected.size());
            for (const auto& entry : expected) REQUIRE(table.get(entry.first) == entry.second);
            REQUIRE_FALSE(table.contains("absent"));
        }
        REQUIRE(table.keys().size() == expected.size());
        REQUIRE(table.values().size() == expected.size());
    }
}
