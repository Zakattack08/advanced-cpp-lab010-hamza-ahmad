#include "project/hash_table.h"

#include <cmath>
#include <stdexcept>
#include <utility>

HashTable::HashTable(std::size_t capacity)
    : table_(capacity), size_(0) {
    if (capacity == 0) {
        throw std::invalid_argument("Hash table capacity must be greater than zero");
    }
    for (std::size_t i = 0; i < table_.size(); ++i) {
        table_[i] = HashEntry();
    }
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
    std::size_t index = hashFunction(key);
    std::size_t original = index;
    std::size_t step = 1;

    while (table_[index].occupied && table_[index].key != key) {
        index = (original + step * step) % table_.size();
        ++step;
    }
    return index;
}

std::size_t HashTable::getIndex(const std::string& key) const {
    const std::size_t original = hashFunction(key);

    for (std::size_t step = 0; step < table_.size(); ++step) {
        const std::size_t index = (original + step * step) % table_.size();
        const HashEntry& entry = table_[index];
        if (entry.occupied && !entry.deleted && entry.key == key) {
            return index;
        }
    }

    return table_.size();
}

bool HashTable::insert(const std::string& key, int value) {
    if (contains(key))
        return false; //if key already exists operation fails

    if ((loadFactor() + static_cast<double>(1/table_.size())) >= 0.5) 
        rehash(); //if table will be 50% or more full after the insertion rehash

    //create and insert new entry
    table_[probe(key)] = HashEntry {key, value, true}; 
    size_++;
    return true;
}

bool HashTable::remove(const std::string& key) {
    const std::size_t index = getIndex(key);
    if (index != table_.size()) {
        table_[index].deleted = true;
        --size_;
        return true;
    }

    return false;
}

bool HashTable::contains(const std::string& key) const {
    return getIndex(key) != table_.size();
}

int HashTable::get(const std::string& key) const {
    const std::size_t index = getIndex(key);
    if (index != table_.size()) {
        return table_[index].value;
    }

    throw std::logic_error("The key \"" + key + "\" does not exist");
}

bool HashTable::update(const std::string& key, int newValue) {
    const std::size_t index = getIndex(key);
    if (index != table_.size()) {
        table_[index].value = newValue;
        return true;
    }
    return false;
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
    std::vector<HashEntry> temp(table_);
    table_ = std::vector<HashEntry> (nextPrime(table_.size()*2));
    for (HashEntry entry : temp) {
        if (entry.occupied) table_[probe(entry.key)] = entry;
    }
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
    for (std::size_t i = 3; i * i <= value; i += 2) {
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
