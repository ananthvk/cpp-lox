#pragma once
#include "chunk.hpp"
#include "function.hpp"
#include "hashmap.hpp"
#include "object.hpp"
#include "table.hpp"

/**
 * Represents a runtime class. Use the allocator class to create new objects.
 */
class ObjectClass : public Object
{
    ObjectString *name_;
    Value init_method;
    StringValueTable methods_;

    ObjectClass(ObjectString *name) : name_(name) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::CLASS; }

    auto name() const -> ObjectString * { return name_; }

    auto methods() -> StringValueTable & { return methods_; }

    auto get_init_method() -> Value & { return init_method; }

    auto operator==(const ObjectClass &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectClass(const ObjectClass &other) = delete;
    ObjectClass &operator=(const ObjectClass &other) = delete;

    // Move not allowed
    ObjectClass(ObjectClass &&other) noexcept = delete;
    ObjectClass &operator=(ObjectClass &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};

/**
 * Represents a runtime instance. Use the allocator class to create new objects. An instance has a
 * hash table that has a list of fields this instance contains. And a reference to the class that
 * it's an instance of
 */
class ObjectInstance : public Object
{
  public:
  private:
    ObjectClass *class_;
    StringValueTable fields;

    ObjectInstance(ObjectClass *class_) : class_(class_) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::INSTANCE; }

    auto get_class() const -> ObjectClass * { return class_; }

    auto get_fields() -> StringValueTable & { return fields; }

    auto operator==(const ObjectInstance &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectInstance(const ObjectInstance &other) = delete;
    ObjectInstance &operator=(const ObjectInstance &other) = delete;

    // Move not allowed
    ObjectInstance(ObjectInstance &&other) noexcept = delete;
    ObjectString &operator=(ObjectInstance &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};

/**
 * Represents a runtime bound method. Use the allocator class to create new objects. A bound method
 * wraps a receiver and a closure together. The receiver is always an ObjectInstance*, but is stored
 * as a Value to avoid tedious conversions from pointer to value. A bound method tracks the instance
 * from which it was created. Inside a bound method, `this` resolveds to the receiver value (an
 * ObjectInstance*)
 */
class ObjectBoundMethod : public Object
{
  private:
    Value receiver_;
    ObjectClosure *method_;

    ObjectBoundMethod(Value receiver, ObjectClosure *method) : receiver_(receiver), method_(method)
    {
    }

  public:
    auto get_type() const -> ObjectType override { return ObjectType::BOUND_METHOD; }

    auto receiver() -> Value { return receiver_; }

    auto method() -> ObjectClosure * { return method_; }

    auto operator==(const ObjectBoundMethod &other) const -> bool { return this == &other; }

    // Copy not allowed
    ObjectBoundMethod(const ObjectInstance &other) = delete;
    ObjectBoundMethod &operator=(const ObjectInstance &other) = delete;

    // Move not allowed
    ObjectBoundMethod(ObjectInstance &&other) noexcept = delete;
    ObjectString &operator=(ObjectBoundMethod &&other) noexcept = delete;

    friend class Allocator;
    friend class Compiler;
};
