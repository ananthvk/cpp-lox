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
    NATIVE_FUNCTION
};

class Object
{
  public:
    virtual auto get_type() const -> ObjectType = 0;

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
