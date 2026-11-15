#include "project/hash_table.h"

#include <cmath>
#include <stdexcept>
#include <utility>

HashTable::HashTable(std::size_t capacity)
    : table_(capacity), size_(0) {
    if (capacity == 0) {
        throw std::invalid_argument("Hash table capacity must be greater than zero");
    }
    table_.resize(nextPrime(capacity));
}

HashTable::~HashTable() = default;

std::size_t HashTable::hashFunction(const std::string& key) const {
    std::size_t hash = 2166136261u;
    for (unsigned char ch : key) {
        hash ^= ch;
        hash *= 16777619u;
    }
    return hash % table_.size();
}

std::size_t HashTable::probe(const std::string& key) const {
    const std::size_t original = hashFunction(key);
    std::size_t firstDeleted = table_.size();
    // Tombstones preserve search paths; remember one but keep checking for duplicates.
    for (std::size_t step = 0; step < table_.size(); ++step) {
        const std::size_t index = (original + step * step) % table_.size();
        const auto& entry = table_[index];
        if (!entry.occupied) {
            return firstDeleted == table_.size() ? index : firstDeleted;
        }
        if (!entry.deleted && entry.key == key) {
            return index;
        }
        if (entry.deleted && firstDeleted == table_.size()) {
            firstDeleted = index;
        }
    }
    return firstDeleted;
}

bool HashTable::insert(const std::string& key, int value) {
    std::size_t index = probe(key);
    if (index < table_.size() && table_[index].occupied && !table_[index].deleted) {
        return false;
    }
    // Prime capacity and load below 1/2 guarantee space along a quadratic probe.
    if (index == table_.size() || size_ + 1 > table_.size() / 2) {
        rehash();
        index = probe(key);
    }
    table_[index] = HashEntry(key, value, true, false);
    ++size_;
    return true;
}

bool HashTable::remove(const std::string& key) {
    if (!contains(key)) {
        return false;
    }
    table_[probe(key)].deleted = true;
    --size_;
    return true;
}

bool HashTable::contains(const std::string& key) const {
    const std::size_t index = probe(key);
    return index < table_.size() && table_[index].occupied && !table_[index].deleted
        && table_[index].key == key;
}

int HashTable::get(const std::string& key) const {
    if (!contains(key)) {
        throw std::out_of_range("Hash table key not found");
    }
    return table_[probe(key)].value;
}

bool HashTable::update(const std::string& key, int newValue) {
    if (!contains(key)) {
        return false;
    }
    table_[probe(key)].value = newValue;
    return true;
}

std::size_t HashTable::size() const noexcept {
    return size_;
}

std::size_t HashTable::capacity() const noexcept {
    return table_.size();
}

bool HashTable::empty() const noexcept {
    return size_ == 0;
}

double HashTable::loadFactor() const noexcept {
    return static_cast<double>(size_) / static_cast<double>(table_.size());
}

std::vector<std::string> HashTable::keys() const {
    std::vector<std::string> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.key);
        }
    }
    return result;
}

std::vector<int> HashTable::values() const {
    std::vector<int> result;
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            result.push_back(entry.value);
        }
    }
    return result;
}

void HashTable::rehash() {
    HashTable replacement(nextPrime(table_.size() * 2));
    for (const auto& entry : table_) {
        if (entry.occupied && !entry.deleted) {
            replacement.insert(entry.key, entry.value);
        }
    }
    table_.swap(replacement.table_);
    std::swap(size_, replacement.size_);
}

bool HashTable::isPrime(std::size_t value) const {
    if (value < 2) {
        return false;
    }
    if (value == 2) {
        return true;
    }
    if (value % 2 == 0) {
        return false;
    }
    for (std::size_t i = 3; i <= value / i; i += 2) {
        if (value % i == 0) {
            return false;
        }
    }
    return true;
}

std::size_t HashTable::nextPrime(std::size_t value) const {
    if (value <= 2) {
        return 2;
    }
    std::size_t candidate = value % 2 == 0 ? value + 1 : value;
    while (!isPrime(candidate)) {
        candidate += 2;
    }
    return candidate;
}
