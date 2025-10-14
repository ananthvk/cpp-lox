#pragma once
#include "chunk.hpp"
#include "object.hpp"
#include "value.hpp"

enum class FunctionType
{
    FUNCTION,
    SCRIPT
};

class VM;

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
// The return value is a pair, first contains the value returned. And second is set to true if the
// call executed successfuly, false otherwise.
typedef std::pair<Value, bool> (*NativeFunction)(VM *vm, int argCount, Value *args);

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

/**
 * Represents a runtime closure. Use the allocator class to create new objects.
 */
class ObjectClosure : public Object
{
    ObjectFunction *function;

    ObjectClosure(ObjectFunction *function) : function(function) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::CLOSURE; }

    auto get() const -> ObjectFunction * { return function; }

    auto operator==(const ObjectClosure &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectClosure(const ObjectString &other) = delete;
    ObjectClosure &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectClosure(ObjectClosure &&other) noexcept = delete;
    ObjectString &operator=(ObjectClosure &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};