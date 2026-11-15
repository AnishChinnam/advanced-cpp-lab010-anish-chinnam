#include "project/hash_table.h"
#include <iostream>

int main() {
    HashTable table;
    table.insert("apple", 10);
    table.insert("banana", 20);
    table.update("apple", 15);
    std::cout << "apple: " << table.get("apple") << '\n';
    table.remove("banana");
    std::cout << "Size: " << table.size() << '\n';
}
