#pragma once
#include "function.hpp"
#include "hashmap.hpp"
#include "object.hpp"
#include "utils.hpp"
#include <stdexcept>
#include <vector>

/**
 * For now, this allocator class just creates the object and returns it's address
 * Later, it can be improved to an arena allocator / pool allocator
 */
class Allocator
{
  private:
    std::vector<Object *> objs;

    // This is basically an ObjectString, but with a different name. Idk why I have this here
    struct InternedString
    {
        const char *str;
        size_t length;
        uint32_t hash;
    };

    struct InternedStringHasher
    {
        auto operator()(InternedString str) const -> size_t { return str.hash; }
    };

    struct InternedStringCmp
    {
        auto operator()(InternedString first, InternedString second) const -> size_t
        {
            if (first.length != second.length)
                return false;
            if (first.hash != second.hash)
                return false;
            return strncmp(first.str, second.str, first.length) == 0;
        }
    };

    HashMap<InternedString, ObjectString *, InternedStringHasher, InternedStringCmp>
        interned_strings;


  public:
    /**
     * Storage type defines if the allocator needs to allocate memory to hold a copy of the data or
     * not.
     */
    enum class StorageType
    {
        DYNAMIC,
        TAKE_OWNERSHIP
    };

    auto hash_string(const char *str, size_t length) const -> uint32_t
    {
        // FNV-1a hash function from the book
        uint32_t hash = 2166136261u;
        for (size_t i = 0; i < length; i++)
        {
            hash ^= (uint8_t)str[i];
            hash *= 16777619;
        }
        return hash;
    }

    auto check_interned(const char *str, size_t length) -> std::pair<ObjectString *, uint32_t>
    {
        // Check if the string is already interned
        uint32_t hash = hash_string(str, length);
        InternedString characters{str, length, hash};
        auto interned_objstring = interned_strings.get(characters);

        /*
        if (interned_objstring)
        {
            fmt::println("Interned string found existing string at {} \"{}\"",
                         static_cast<void *>(interned_objstring.value()),
                         std::string_view{str, length});
        }
        */

        return std::make_pair(interned_objstring.value_or(nullptr), hash);
    }

    /**
     * If `storage_type` is `StorageType::DYNAMIC`, the allocator creates
     * another buffer to hold the copy of the string, and that buffer is managed by the allocator.
     *
     * If `storage_type` is `StorageType::TAKE_OWNERSHIP`, the allocator take ownership of the
     * passed buffer, and manages it. The buffer must be created only using new[], since the
     * allocator will call *delete[]* to destroy the buffer.
     */
    auto intern_string(const char *str, size_t length,
                       StorageType storage_type = StorageType::DYNAMIC) -> ObjectString *
    {
        auto [interned_string, hash] = check_interned(str, length);

        if (interned_string)
        {
            if (storage_type == StorageType::DYNAMIC)
                return interned_string;
            else
            {
                // Delete the passed string since it already exists in memory
                delete[] str;
                return interned_string;
            }
        }

        if (storage_type == StorageType::DYNAMIC)
        {
            // TODO: Use placement new (no flexible array members in C++) to create the ObjectString
            // & the buffer at the same location.
            char *buffer = new char[length + 1];
            memcpy(buffer, str, length);
            buffer[length] = '\0';
            auto obj = new ObjectString(buffer, length, hash);
            objs.push_back(obj);

            // Add the string to the string pool
            InternedString intern{buffer, length, hash};
            interned_strings.insert(intern, obj);

            return obj;
        }
        else if (storage_type == StorageType::TAKE_OWNERSHIP)
        {
            auto obj = new ObjectString(str, length, hash);
            objs.push_back(obj);

            // Add the string to the string pool
            InternedString intern{str, length, hash};
            interned_strings.insert(intern, obj);

            return obj;
        }
        throw std::logic_error("Invalid storage type");
    }

    auto intern_string(std::string_view sv, StorageType storage_type = StorageType::DYNAMIC)
        -> ObjectString *
    {
        return intern_string(sv.data(), sv.size(), storage_type);
    }

    auto new_function(int arity, std::string_view name) -> ObjectFunction *
    {
        auto chunk = std::make_unique<Chunk>();
        auto interned_name = intern_string(name);
        auto obj = new ObjectFunction(arity, std::move(chunk), interned_name);
        objs.push_back(obj);
        return obj;
    }

    auto new_native_function(int arity, NativeFunction function) -> ObjectNativeFunction *
    {
        auto obj = new ObjectNativeFunction(arity, function);
        objs.push_back(obj);
        return obj;
    }

    // Note: Removed static strings, and instead opted to intern all strings so that checking for
    // equality becomes easier

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
        for (auto obj : objs)
        {
            delete obj;
        }
    }
};
