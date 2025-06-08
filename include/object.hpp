#pragma once
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string_view>
#include <vector>


/**
 * Temporary fix for avoiding leaks
 */
class ObjectString;
extern std::vector<ObjectString *> objs;


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
    size_t length;
    char *data;


  public:
    /**
     * Default constructor, creates an empty string
     */
    ObjectString() : length(0), data(nullptr) {}

    /**
     * Makes a copy of the characters of the passed string view
     */
    ObjectString(std::string_view sv)
    {
        data = new char[sv.size() + 1];
        sv.copy(data, sv.size());
        data[sv.size()] = '\0';
        length = sv.size();
    }

    auto get_type() const -> ObjectType override { return ObjectType::STRING; }

    auto size() const -> size_t { return length; }

    auto get() -> char * { return data; }

    auto get() const -> const char * { return data; }

    auto get_sv() const -> std::string_view { return std::string_view{data, length}; }

    auto operator==(const ObjectString &other) const -> bool
    {
        if (length != other.length)
            return false;
        return memcmp(data, other.data, length) == 0;
    }

    /**
     * Creates an ObjectString from an already existing buffer. Note: This function takes
     * ownership of the buffer. Also the buffer should only be allocated using new
     */
    ObjectString(char *buffer, size_t length) : length(length), data(buffer) {}

    ~ObjectString()
    {
        if (data)
        {
            delete[] data;
            data = nullptr;
            length = 0;
        }
    }

    // TODO: Implement rule of 5
    // Copy not allowed
    ObjectString(const ObjectString &other) = delete;
    ObjectString &operator=(const ObjectString &other) = delete;

    // Move not allowed
    ObjectString(ObjectString &&other) noexcept = delete;
    ObjectString &operator=(ObjectString &&other) noexcept = delete;
};
