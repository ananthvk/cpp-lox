#include "value.hpp"
#include "classes.hpp"
#include "function.hpp"
#include "list.hpp"
#include "map.hpp"
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
        case ObjectType::CLASS:
            return std::string("<class ") +
                   std::string(static_cast<ObjectClass *>(data.o)->name()->get()) +
                   std::string(">");
        case ObjectType::UPVALUE:
            return "<upvalue>";
        case ObjectType::NATIVE_FUNCTION:
            return std::string("<native function>");
        case ObjectType::INSTANCE:
            return fmt::format("<instance of {} at {:p}>",
                               static_cast<ObjectInstance *>(data.o)->get_class()->name()->get(),
                               static_cast<void *>(data.o));
        case ObjectType::BOUND_METHOD:
            return fmt::format("<bound method of {}>",
                               static_cast<ObjectBoundMethod *>(data.o)->receiver().to_string());
        case ObjectType::LIST:
        {
            std::string result = "";
            auto &values = static_cast<ObjectList *>(data.o)->get_values();
            bool first_one = true;
            for (auto value : values)
            {
                if (first_one)
                    result += value.to_string();
                else
                    result += ", " + value.to_string();
                first_one = false;
            }
            return fmt::format("[{}]", result);
        }
        case ObjectType::MAP:
        {
            std::string result = "";
            auto &slots = static_cast<ObjectMap *>(data.o)->get_table().get_slots();
            bool first_one = true;
            for (auto &slot : slots)
            {
                if (slot.state == ValueValueTable::Slot::State::FILLED)
                {
                    if (first_one)
                        result += slot.key.to_string() + ": " + slot.value.to_string();
                    else
                        result += ", " + slot.key.to_string() + ": " + slot.value.to_string();
                    first_one = false;
                }
            }
            return fmt::format("{{{}}}", result);
        }
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
        case ObjectType::BOUND_METHOD:
        {
            auto bound1 = static_cast<ObjectBoundMethod *>(as_object());
            auto bound2 = static_cast<ObjectBoundMethod *>(other.as_object());
            return bound1->method() == bound2->method() && bound1->receiver() == bound2->receiver();
        }
        // Checks if both functions point to the same in memory object
        case ObjectType::FUNCTION:
        case ObjectType::NATIVE_FUNCTION:
        case ObjectType::CLASS:
        case ObjectType::INSTANCE:
        case ObjectType::CLOSURE:
        case ObjectType::LIST:
        case ObjectType::MAP:
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

auto Value::hash_code() const -> int64_t
{
    switch (type)
    {
    case ValueType::NIL:
        return NIL_HASH_VALUE;
    case ValueType::BOOLEAN:
        return as_bool() ? 1 : 0;
    case ValueType::NUMBER_INT:
        return hash_int(as_integer());
    case Value::NUMBER_REAL:
        return hash_double(as_real());
    case ValueType::OBJECT:
        return as_object()->hash_code();
    default:
        throw std::logic_error("invalid object type");
    }
}