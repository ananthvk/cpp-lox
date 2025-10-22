#pragma once
#include "chunk.hpp"
#include "hashmap.hpp"
#include "object.hpp"
#include "table.hpp"

/**
 * Represents a runtime class. Use the allocator class to create new objects.
 */
class ObjectClass : public Object
{
    ObjectString *name_;
    StringValueTable methods_;

    ObjectClass(ObjectString *name) : name_(name) {}

  public:
    auto get_type() const -> ObjectType override { return ObjectType::CLASS; }

    auto name() const -> ObjectString * { return name_; }

    auto methods() -> StringValueTable & { return methods_; }

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
 * Represents a runtime class. Use the allocator class to create new objects. An instance has a hash
 * table that has a list of fields this instance contains. And a reference to the class that it's an
 * instance of
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
