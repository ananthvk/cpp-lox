#pragma once
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

const int64_t NIL_HASH_VALUE = 0xDEADC0DE;
const int64_t NAN_HASH_VALUE = 0xC0DE10ADED;

auto inline hash_string(const char *str, size_t length) -> uint32_t
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

// Based on Splitmix64
auto inline hash_double(double d) -> int64_t
{
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wfloat-equal"
    if (d == 0.0 || d == -0.0)
        return 0; // Normalize -0.0
#pragma GCC diagnostic pop
    if (isnan(d) || isinf(d))
        return NAN_HASH_VALUE;
    uint64_t x = 0;
    memcpy(&x, &d, sizeof(x));
    uint64_t z = (x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    z = z ^ (z >> 31);
    int64_t h = static_cast<int64_t>(z);
    if (h == -1)
        return -2;
    return h;
}

// Also based on splitmix64, hashes integers too so that clustering can be avoided
auto inline hash_int(int64_t v) -> int64_t
{
    uint64_t x = v;
    uint64_t z = (x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    z = z ^ (z >> 31);
    int64_t h = static_cast<int64_t>(z);
    if (h == -1)
        return -2;
    return h;
}

// Hash of a pointer is identity
auto inline hash_pointer(const void *p) -> int64_t
{
    auto h = static_cast<int64_t>(reinterpret_cast<uintptr_t>(p));
    if (h == -1)
        return -2;
    return h;
}