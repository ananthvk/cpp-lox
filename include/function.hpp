#pragma once
#include "chunk.hpp"
#include "object.hpp"

/**
 * Represents a runtime object. Use the allocator class to create new objects.
 */
class ObjectFunction : public Object
{
    int arity_;
    std::unique_ptr<Chunk> chunk;
    ObjectString *name_;

    ObjectFunction(int arity, std::unique_ptr<Chunk> chunk, ObjectString *name)
        : arity_(arity), chunk(std::move(chunk)), name_(name)
    {
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::FUNCTION; }

    auto arity() const -> size_t { return arity_; }

    auto name() const -> ObjectString * { return name_; }

    auto get() const -> Chunk * { return chunk.get(); }

    auto operator==(const ObjectFunction &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectFunction(const ObjectString &other) = delete;
    ObjectFunction &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectFunction(ObjectFunction &&other) noexcept = delete;
    ObjectString &operator=(ObjectFunction &&other) noexcept = delete;

    friend class Allocator;
};