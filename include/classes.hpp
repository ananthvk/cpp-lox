#pragma once
#include "chunk.hpp"
#include "object.hpp"

/**
 * Represents a runtime class. Use the allocator class to create new objects.
 */
class ObjectClass : public Object
{
    ObjectString *name_;

    ObjectClass(ObjectString *name) : name_(name) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::CLASS; }

    auto name() const -> ObjectString * { return name_; }

    auto operator==(const ObjectClass &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectClass(const ObjectClass &other) = delete;
    ObjectClass &operator=(const ObjectClass &other) = delete;

    // Move not allowed
    ObjectClass(ObjectClass &&other) noexcept = delete;
    ObjectString &operator=(ObjectClass &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};
