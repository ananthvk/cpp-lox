#include "value.hpp"
#include "function.hpp"
#include "object.hpp"

auto Value::to_string() const -> std::string
{
    switch (type)
    {
    case Value::NIL:
        return "nil";
    case Value::NUMBER_REAL:
        return fmt::format("{:.16g}", data.d);
    case Value::NUMBER_INT:
        return std::to_string(data.i);
    case Value::OBJECT:
    {
        switch (data.o->get_type())
        {
        case ObjectType::STRING:
            return std::string(static_cast<ObjectString *>(data.o)->get());
        case ObjectType::FUNCTION:
        {
            auto name = static_cast<ObjectFunction *>(data.o)->name()->get();
            if (name == "")
                return "<script>";
            else
                return std::string("<function ") + std::string(name) + std::string(">");
            break;
        }
        case ObjectType::CLOSURE:
        {
            auto name = static_cast<ObjectClosure *>(data.o)->get()->name()->get();
            if (name == "")
                return "<script>";
            else
                return std::string("<function ") + std::string(name) + std::string(">");
            break;
        }
        case ObjectType::UPVALUE:
            return "<upvalue>";
        case ObjectType::NATIVE_FUNCTION:
            return std::string("<native function>");
        default:
            throw std::logic_error("invalid object type");
            break;
        }
        break;
    }

    case Value::BOOLEAN:
        if (data.b)
            return "true";
        else
            return "false";
        break;
    }
    throw std::logic_error("Invalid value type passed to to_string");
}

auto Value::operator==(Value other) const -> bool
{
    if (type != other.type)
        return false;
    switch (type)
    {
    case ValueType::NIL:
        return true;
    case ValueType::BOOLEAN:
        return as_bool() == other.as_bool();
    case ValueType::NUMBER_INT:
        return as_integer() == other.as_integer();
    case ValueType::OBJECT:
        switch (data.o->get_type())
        {
        case ObjectType::STRING:
            return *as_string() == *other.as_string();
        // Checks if both functions point to the same in memory object
        case ObjectType::FUNCTION:
            return as_object() == other.as_object();
        case ObjectType::CLOSURE:
            return as_object() == other.as_object();
        case ObjectType::NATIVE_FUNCTION:
            return as_object() == other.as_object();
        case ObjectType::UPVALUE:
            // Two upvalues are equal if they point to the same location
            return static_cast<ObjectUpvalue *>(as_object())->get() ==
                   static_cast<ObjectUpvalue *>(other.as_object())->get();
        }
        return false;
    case Value::NUMBER_REAL:
        // Note: Comparing doubles like this is incorrect due to floating point precision errors
        // TODO: Fix this by either defining an epsilon, or do not allow == between doubles
        return fabs(as_real() - other.as_real()) < EPSILON;
    }
    return false;
}