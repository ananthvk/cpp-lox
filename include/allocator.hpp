#pragma once
#include "object.hpp"
#include <stdexcept>
#include <vector>

/**
 * For now, this allocator class just creates the object and returns it's address
 * Later, it can be improved to an arena allocator / pool allocator
 */
class Allocator
{
  private:
    std::vector<const char *> buffers;
    std::vector<Object *> objs;

  public:
    /**
     * Storage type defines if the allocator needs to allocate memory to hold a copy of the data or
     * not.
     */
    enum class StorageType
    {
        STATIC,
        DYNAMIC,
        TAKE_OWNERSHIP
    };

    /**
     * If `storage_type` is `StorageType::STATIC`, the allocator assumes that the passed string is a
     * constant string and is not freed within the lifetime of
     * ObjectString's usage.
     *
     * If `storage_type` is `StorageType::DYNAMIC`, the allocator creates
     * another buffer to hold the copy of the string, and that buffer is managed by the allocator.
     *
     * If `storage_type` is `StorageType::TAKE_OWNERSHIP`, the allocator take ownership of the
     * passed buffer, and manages it. The buffer must be created only using new[], since the
     * allocator will call *delete[]* to destroy the buffer.
     */
    auto allocate_string(const char *str, size_t length,
                         StorageType storage_type = StorageType::DYNAMIC) -> ObjectString *
    {
        if (storage_type == StorageType::DYNAMIC)
        {
            char *buffer = new char[length + 1];
            memcpy(buffer, str, length);
            buffer[length] = '\0';
            auto obj = new ObjectString(buffer, length);
            objs.push_back(obj);
            buffers.push_back(buffer);
            return obj;
        }
        else if (storage_type == StorageType::TAKE_OWNERSHIP)
        {
            auto obj = new ObjectString(str, length);
            objs.push_back(obj);
            buffers.push_back(str);
            return obj;
        }
        else if (storage_type == StorageType::STATIC)
        {
            auto obj = new ObjectString(str, length);
            objs.push_back(obj);
            return obj;
        }
        throw std::logic_error("Invalid storage type");
    }

    auto allocate_string(std::string_view sv, StorageType storage_type = StorageType::DYNAMIC)
        -> ObjectString *
    {
        return allocate_string(sv.data(), sv.size(), storage_type);
    }

    /*
    // TODO: Manually freeing strings is not yet allowed, the allocator clears it at the end
    auto free_string(ObjectString *obj) -> void
    {
        delete[] obj->data;
        delete obj;
    }
    */

    ~Allocator()
    {
        for (auto buffer : buffers)
        {
            delete[] buffer;
        }
        for (auto obj : objs)
        {
            delete obj;
        }
    }
};
