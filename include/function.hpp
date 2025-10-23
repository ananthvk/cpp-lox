#pragma once
#include "chunk.hpp"
#include "object.hpp"
#include "value.hpp"

enum class FunctionType
{
    FUNCTION,
    SCRIPT,
    METHOD,
    INITIALIZER
};

class VM;

/**
 * Represents a runtime function. Use the allocator class to create new objects.
 */
class ObjectFunction : public Object
{
    int arity_;
    int upvalue_count_;
    std::unique_ptr<Chunk> chunk;
    ObjectString *name_;

    ObjectFunction(int arity, std::unique_ptr<Chunk> chunk, ObjectString *name)
        : arity_(arity), upvalue_count_(0), chunk(std::move(chunk)), name_(name)
    {
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::FUNCTION; }

    auto arity() const -> size_t { return arity_; }

    auto upvalue_count() const -> int { return upvalue_count_; }

    auto name() const -> ObjectString * { return name_; }

    auto get() const -> Chunk * { return chunk.get(); }

    auto operator==(const ObjectFunction &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectFunction(const ObjectFunction &other) = delete;
    ObjectFunction &operator=(const ObjectFunction &other) = delete;

    // Move not allowed
    ObjectFunction(ObjectFunction &&other) noexcept = delete;
    ObjectFunction &operator=(ObjectFunction &&other) noexcept = delete;

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
    ObjectNativeFunction(const ObjectNativeFunction &other) = delete;
    ObjectNativeFunction &operator=(const ObjectNativeFunction &other) = delete;

    // Move not allowed
    ObjectNativeFunction(ObjectNativeFunction &&other) noexcept = delete;
    ObjectNativeFunction &operator=(ObjectNativeFunction &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};

/**
 * Represents a runtime upvalue object. Use the allocator class to create new objects.
 */
class ObjectUpvalue : public Object
{
  public:
    ObjectUpvalue *next;
    Value closed;

  private:
    Value *location;

    ObjectUpvalue(Value *slot) : next(nullptr), location(slot) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::UPVALUE; }

    auto get() const -> Value * { return location; }

    auto set(Value *slot) { location = slot; }

    auto operator==(const ObjectUpvalue &other) const -> bool { return location == other.location; }

    // Copy not allowed
    ObjectUpvalue(const ObjectUpvalue &other) = delete;
    ObjectUpvalue &operator=(const ObjectUpvalue &other) = delete;

    // Move not allowed
    ObjectUpvalue(ObjectUpvalue &&other) noexcept = delete;
    ObjectUpvalue &operator=(ObjectUpvalue &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};

/**
 * Represents a runtime closure. Use the allocator class to create new objects.
 */
class ObjectClosure : public Object
{
    ObjectFunction *function;
    std::vector<ObjectUpvalue *> upvalues;
    int upvalue_count_;

    ObjectClosure(ObjectFunction *function) : function(function)
    {
        upvalues.resize(function->upvalue_count());
        upvalue_count_ = function->upvalue_count();
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::CLOSURE; }

    auto get() const -> ObjectFunction * { return function; }

    auto get_upvalues() -> std::vector<ObjectUpvalue *> & { return upvalues; }

    auto upvalue_count() const -> int { return upvalue_count_; }

    auto operator==(const ObjectClosure &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectClosure(const ObjectClosure &other) = delete;
    ObjectClosure &operator=(const ObjectClosure &other) = delete;

    // Move not allowed
    ObjectClosure(ObjectClosure &&other) noexcept = delete;
    ObjectClosure &operator=(ObjectClosure &&other) noexcept = delete;

    friend class Allocator;
};
