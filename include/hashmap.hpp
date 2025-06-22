#ifndef A_HASHMAP_H
#define A_HASHMAP_H
#include <algorithm>
#include <iostream>
#include <optional>
#include <stdint.h>
#include <vector>

// TODO: Later change the implementation to use a raw buffer + placement new
//
/*
 * HashMap
 * =========
 * This is a implementation of a hash table, using open addressing collision resolution
 * Design choices
 * 1) No iterators - For simplicity and faster access, this implementation does not implement
 * iterators 2) Supports three main operations, along with three other operations
 *     * get(Key) - Returns an optional value
 *     * insert(Key, Value) - Inserts the Key-Value pair into the hash table
 *     * erase(Key) - Deletes the key if it exists
 *     * contains(Key) - Returns true if the key exists
 *     * size() - Returns the number of keys in the hash table
 *     * clear() - Remove all keys from the table
 * 3) To improve cache efficiency, the table uses open addressing collision resolution mechanism
 * 4) The current implementation requires Key and Value to be DefaultConstructible
 */
template <typename Key, typename Value, typename Hash = std::hash<Key>,
          typename KeyEqual = std::equal_to<Key>>
class HashMap
{
  private:
    struct Slot
    {
        enum class State : uint8_t
        {
            FILLED,
            EMPTY,
            TOMBSTONE
        };
        Key key;
        Value value;
        State state;

        Slot() : key(), value(), state(State::EMPTY) {}
    };

    KeyEqual key_equal_;
    Hash hasher_;
    size_t size_, tombstones_;
    double max_load_factor_, growth_factor_;
    std::vector<Slot> slots;

    auto find_slot(std::vector<Slot> &table_slots, const Key &key) -> Slot &
    {
        uint64_t hash = hasher_(key);
        uint64_t index = hash % table_slots.size();
        Slot *tombstone = nullptr;

        while (true)
        {
            Slot &slot = table_slots[index];
            if (key_equal_(slot.key, key))
                return slot;
            if (slot.state == Slot::State::EMPTY)
            {
                // If there is no entry, return a tombstone that was found earlier (if any) in the
                // probe sequence to reduce the number of tombstones
                if (tombstone != nullptr)
                    return *tombstone;
                return slot;
            }
            if (slot.state == Slot::State::TOMBSTONE)
                tombstone = &slot;
            index = (index + 1) % table_slots.size();
        }
    }

    auto grow() -> void
    {
        auto new_size = size_ + 1 + tombstones_;
        if (new_size > static_cast<size_t>(static_cast<double>(slots.size()) * max_load_factor_))
        {
            auto new_capacity =
                std::max(static_cast<size_t>(growth_factor_ * static_cast<double>(slots.size())),
                         MIN_TABLE_SIZE);

            // Rehash the elements
            std::vector<Slot> new_slots(new_capacity);
            for (size_t i = 0; i < slots.size(); ++i)
            {
                if (slots[i].state == Slot::State::FILLED)
                {
                    Slot &destination = find_slot(new_slots, slots[i].key);
                    destination.value = std::move(slots[i].value);
                    destination.key = slots[i].key;
                    destination.state = slots[i].state;
                }
            }
            // Move the vector
            slots = std::move(new_slots);
            tombstones_ = 0;
        }
    }

  public:
    using key_type = Key;
    using mapped_type = Value;
    using size_type = size_t;
    using difference_type = std::ptrdiff_t;
    using hasher = Hash;
    using key_equal = KeyEqual;

    static constexpr double DEFAULT_MAX_LOAD_FACTOR = 0.75;
    static constexpr double DEFAULT_GROWTH_FACTOR = 2.0;
    static constexpr size_t MIN_TABLE_SIZE = 8;

    HashMap()
        : size_(0), tombstones_(0), max_load_factor_(DEFAULT_MAX_LOAD_FACTOR),
          growth_factor_(DEFAULT_GROWTH_FACTOR)
    {
    }

    auto get(const Key &key) -> std::optional<Value>
    {
        if (!size_)
            return std::nullopt;
        Slot &slot = find_slot(slots, key);
        if (slot.state == Slot::State::FILLED)
        {
            return slot.value;
        }
        return std::nullopt;
    }

    auto get_ref(const Key &key) -> Value &
    {
        Slot &slot = find_slot(slots, key);
        return slot.value;
    }

    /**
     * Returns true if a new key was inserted
     */
    auto insert(const Key &key, const Value &value) -> bool
    {
        grow();

        Slot &slot = find_slot(slots, key);
        if (slot.state == Slot::State::EMPTY)
        {
            slot.key = key;
            slot.value = value;
            slot.state = Slot::State::FILLED;
            size_ += 1;
            return true;
        }
        else if (slot.state == Slot::State::TOMBSTONE)
        {
            slot.key = key;
            slot.value = value;
            slot.state = Slot::State::FILLED;
            size_ += 1;
            tombstones_ -= 1;
            return true;
        }
        else
        {
            slot.value = value;
            return false;
        }
    }

    auto contains(const Key &key) const -> bool
    {
        Slot &slot = find_slot(slots, key);
        return slot.state == Slot::State::FILLED;
    }

    auto erase(const Key &key) -> bool
    {
        if (size_ == 0)
            return false;

        Slot &slot = find_slot(slots, key);
        if (slot.state != Slot::State::FILLED)
            return false;

        // Make the slot a tombstone
        slot.state = Slot::State::TOMBSTONE;
        ++tombstones_;
        --size_;
        // Note: Destructor of Key/Value are not called here, so for now use this table only with
        // POD
        return true;
    }

    auto size() const -> size_type { return size_; }

    auto clear() -> void
    {
        size_ = 0;
        tombstones_ = 0;
        slots.clear();
    }

    auto load_factor() const -> double
    {
        return slots.size()
                   ? (static_cast<double>(size_ + tombstones_) / static_cast<double>(slots.size()))
                   : 0;
    }

    auto max_load_factor() const -> double { return max_load_factor_; }

    auto max_load_factor(double new_max_load_factor) -> void
    {
        if (new_max_load_factor > 1.0f || new_max_load_factor < 0.0f)
            throw std::logic_error("Invalid Max load factor, it should be between 0 and 1");
        max_load_factor_ = new_max_load_factor;
    }

    auto growth_factor() const -> double { return growth_factor_; }

    auto growth_factor(double new_growth_factor) -> void
    {
        if (new_growth_factor > 1.0f)
            throw std::logic_error("Growth factor is greater than 1");
        growth_factor_ = new_growth_factor;
    }
};

#endif // A_HASHMAP_H