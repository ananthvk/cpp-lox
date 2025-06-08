#pragma once
#include <stddef.h>
#include <stdint.h>

enum class ObjectType : uint8_t
{
    STRING
};

class Object
{
  public:
    virtual auto get_type() const -> ObjectType = 0;

    virtual ~Object() {}
};

class ObjectString : public Object
{
  public:
    int length;
    char *data;

    auto get_type() const -> ObjectType override { return ObjectType::STRING; }
};