#pragma once
#include "chunk.hpp"
#include "object.hpp"
#include "value.hpp"

enum class FunctionType
{
    FUNCTION,
    SCRIPT
};

/**
 * Represents a runtime function. Use the allocator class to create new objects.
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

    auto set_name(ObjectString *new_name) -> void { name_ = new_name; }

    auto get() const -> Chunk * { return chunk.get(); }

    auto operator==(const ObjectFunction &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectFunction(const ObjectString &other) = delete;
    ObjectFunction &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectFunction(ObjectFunction &&other) noexcept = delete;
    ObjectString &operator=(ObjectFunction &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};

// Function pointer to a native function
typedef Value (*NativeFunction)(int argCount, Value *args);

/**
 * Represents a native function. Use the allocator class to create new objects.
 */
class ObjectNativeFunction : public Object
{
    int arity_;
    NativeFunction function;

    ObjectNativeFunction(int arity, NativeFunction function) : arity_(arity), function(function) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::NATIVE_FUNCTION; }

    auto arity() const -> size_t { return arity_; }

    auto get() const -> NativeFunction { return function; }

    auto operator==(const ObjectNativeFunction &other) const -> bool
    {
        return this->function == other.function;
    }

    // Copy not allowed
    ObjectNativeFunction(const ObjectString &other) = delete;
    ObjectNativeFunction &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectNativeFunction(ObjectNativeFunction &&other) noexcept = delete;
    ObjectString &operator=(ObjectNativeFunction &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};