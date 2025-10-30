#pragma once
#include <memory>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string_view>
#include <vector>


enum class ObjectType : uint8_t
{
    STRING,
    FUNCTION,
    CLOSURE,
    NATIVE_FUNCTION,
    UPVALUE,
    CLASS,
    INSTANCE,
    BOUND_METHOD,
    LIST,
    MAP
};

class Object
{
  public:
    bool is_marked;

    virtual auto get_type() const -> ObjectType = 0;

    // The hash of the value, should return -1 if the object is not hashable
    // If two objects are equal, they MUST return the same hash value, different objects may return
    // the same hash value
    virtual auto hash_code() const -> int64_t = 0;

    virtual ~Object() {}
};

/**
 * Represents a runtime string. ObjectString is immutable and cannot be constructed externally, to
 * create and manage the object, Allocator class needs to be used
 */
class ObjectString : public Object
{
    const char *data;
    size_t length;
    uint32_t hash_;

    ObjectString(const char *data, size_t length, uint32_t hash)
        : data(data), length(length), hash_(hash)
    {
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::STRING; }

    auto size() const -> size_t { return length; }

    auto hash() const -> uint32_t { return hash_; }

    auto hash_code() const -> int64_t override { return static_cast<int64_t>(hash_); }

    auto get() const -> std::string_view { return std::string_view{data, length}; }

    auto operator==(const ObjectString &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectString(const ObjectString &other) = delete;
    ObjectString &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectString(ObjectString &&other) noexcept = delete;
    ObjectString &operator=(ObjectString &&other) noexcept = delete;

    ~ObjectString() { delete[] data; }

    friend class Allocator;
};

inline auto object_type_to_string(ObjectType type) -> const char *
{
    switch (type)
    {
    case ObjectType::STRING:
        return "STRING";
    case ObjectType::FUNCTION:
        return "FUNCTION";
    case ObjectType::CLOSURE:
        return "CLOSURE";
    case ObjectType::NATIVE_FUNCTION:
        return "NATIVE_FUNCTION";
    case ObjectType::UPVALUE:
        return "UPVALUE";
    case ObjectType::CLASS:
        return "CLASS";
    case ObjectType::INSTANCE:
        return "INSTANCE";
    case ObjectType::BOUND_METHOD:
        return "BOUND_METHOD";
    case ObjectType::LIST:
        return "LIST";
    case ObjectType::MAP:
        return "MAP";
    default:
        return "UNKNOWN";
    }
}