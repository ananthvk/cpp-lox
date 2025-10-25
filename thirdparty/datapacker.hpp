/**
 * @file datapacker.h
 * @brief Utilities for encoding/decoding different data types in various binary formats
 *
 * This file contains various template functions which can be used to encode/decode data types in a
 * portable way. Contains functions to encode/decode little-endian/big-endian, floats (using IEEE754
 * format), strings and arrays.
 * @note
 * - Ensure that buffer has enough length to store all the values to prevent buffer overflows
 * @version 1.0
 * @date 2024-11-14
 * @author Ananthanarayanan Venkitakrishnan
 * Homepage: https://github.com/ananthvk/datapacker
 */
#ifndef A_DATAPACKER_H
#define A_DATAPACKER_H
#include <inttypes.h>
#include <istream>
#include <ostream>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace datapacker
{
// Default maximum number of elements which can be read using the stream api
constexpr size_t DEFAULT_MAX_NUMBER_OF_ELEMENTS = 1000 * 1000;
enum class endian
{
    little = 0,
    big = 1
};

namespace internal
{
template <class...> struct False : std::bool_constant<false>
{
};

template <typename T> struct is_float : std::false_type
{
};

template <typename T> struct is_double : std::false_type
{
};

template <> struct is_float<float> : std::true_type
{
};

template <> struct is_double<double> : std::true_type
{
};

template <> struct is_float<const float> : std::true_type
{
};

template <> struct is_double<const double> : std::true_type
{
};

template <unsigned bits, unsigned expbits> uint64_t pack754(long double f);

template <unsigned bits, unsigned expbits> long double unpack754(uint64_t i);
} // namespace internal

/**
 * All the functions in this namespace operate on raw bytes, and it is necessary that the caller
 * makes sure that the buffers are of sufficient length to prevent overflow
 */
namespace bytes
{
/**
 * @brief Encodes a float in IEEE754 format and stores it in `buffer`
 * @param buffer Pointer to buffer which will be used to store the encoded data
 * @param value Value to be encoded
 * @return Number of bytes written to the buffer
 * @note `buffer` should have size atleast equal to `sizeof(float)`
 */

template <typename T> int encode_le(uint8_t *buffer, T value);

template <typename T> int decode_le(uint8_t *buffer, T &value);

// Note: floats and doubles, when converted to int64_t are encoded in little endian format

inline int encode_float(uint8_t *buffer, float f)
{
    uint64_t encoded = internal::pack754<32, 8>(f);
    uint32_t result = static_cast<uint32_t>(encoded & 0xFFFFFFFF);
    encode_le(buffer, result);
    return sizeof(float);
}

/**
 * @brief Encodes a double in IEEE754 format and stores it in `buffer`
 * @param buffer Pointer to buffer which will be used to store the encoded data
 * @param value Value to be encoded
 * @return Number of bytes written to the buffer
 * @note `buffer` should have size atleast equal to `sizeof(double)`
 */
inline int encode_double(uint8_t *buffer, double f)
{
    uint64_t encoded = internal::pack754<64, 11>(f);
    encode_le(buffer, encoded);
    return sizeof(double);
}

/**
 * @brief Decodes a float stored in IEEE754 format and stores it in `f`
 * @param buffer Pointer to buffer which contains the encoded data
 * @param value Where the decoded value will be stored
 * @return Number of bytes read from the buffer
 * @note `buffer` should have size atleast equal to `sizeof(float)`
 */
inline int decode_float(uint8_t *buffer, float &f)
{
    uint32_t i;
    decode_le(buffer, i);
    f = static_cast<float>(internal::unpack754<32, 8>(static_cast<uint64_t>(i)));
    return sizeof(float);
}

/**
 * @brief Decodes a double stored in IEEE754 format and stores it in `f`
 * @param buffer Pointer to buffer which contains the encoded data
 * @param value Where the decoded value will be stored
 * @return Number of bytes read from the buffer
 * @note `buffer` should have size atleast equal to `sizeof(double)`
 */
inline int decode_double(uint8_t *buffer, double &f)
{
    uint64_t i;
    decode_le(buffer, i);
    f = static_cast<double>(internal::unpack754<64, 11>(i));
    return sizeof(double);
}

/**
 * @brief Encodes a value in big-endian format and stores it in `buffer`
 * @tparam T Type of value to be encoded, `sizeof(T)` bytes are written to the buffer
 * @param buffer Pointer to buffer which will be used to store the encoded data
 * @param value Value to be encoded
 * @return Number of bytes written to the buffer
 * @note `buffer` should have size atleast equal to `sizeof(T)`
 *
 * Example usage:
 * @code
 * uint8_t buffer[4];
 * int x = 32;
 * encode_be(buffer, x);
 * @endcode
 */
template <typename T> inline int encode_be(uint8_t *buffer, T value)
{
    static_assert(std::is_integral<T>::value);
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    buffer[0] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 8)) & 0xFF);
    if constexpr (sizeof(T) >= 2)
    {
        buffer[1] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 16)) & 0xFF);
    }
    if constexpr (sizeof(T) >= 4)
    {
        buffer[2] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 24)) & 0xFF);
        buffer[3] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 32)) & 0xFF);
    }
    if constexpr (sizeof(T) >= 8)
    {
        buffer[4] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 40)) & 0xFF);
        buffer[5] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 48)) & 0xFF);
        buffer[6] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 56)) & 0xFF);
        buffer[7] = static_cast<uint8_t>((value >> (8 * sizeof(T) - 64)) & 0xFF);
    }
    return sizeof(T);
}

/**
 * @brief Encodes a value in little-endian format and stores it in `buffer`
 * @tparam T Type of value to be encoded, `sizeof(T)` bytes are written to the buffer
 * @param buffer Pointer to buffer which will be used to store the encoded data
 * @param value Value to be encoded
 * @return Number of bytes written to the buffer
 * @note `buffer` should have size atleast equal to `sizeof(T)`
 *
 * Example usage:
 * @code
 * uint8_t buffer[4];
 * int x = 32;
 * encode_le(buffer, x);
 * @endcode
 */
template <typename T> inline int encode_le(uint8_t *buffer, T value)
{
    static_assert(std::is_integral<T>::value);
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    buffer[0] = static_cast<uint8_t>(value & 0xFF);
    if constexpr (sizeof(T) >= 2)
    {
        buffer[1] = static_cast<uint8_t>((value >> 8) & 0xFF);
    }
    if constexpr (sizeof(T) >= 4)
    {
        buffer[2] = static_cast<uint8_t>((value >> 16) & 0xFF);
        buffer[3] = static_cast<uint8_t>((value >> 24) & 0xFF);
    }
    if constexpr (sizeof(T) >= 8)
    {
        buffer[4] = static_cast<uint8_t>((value >> 32) & 0xFF);
        buffer[5] = static_cast<uint8_t>((value >> 40) & 0xFF);
        buffer[6] = static_cast<uint8_t>((value >> 48) & 0xFF);
        buffer[7] = static_cast<uint8_t>((value >> 56) & 0xFF);
    }
    return sizeof(T);
}

/**
 * @brief Decodes a little-endian encoded value from the buffer and stores it in `value`, the number
 * of bytes processed depends upon the type `T` of value passed
 * @tparam T Type of value to be decoded, `sizeof(T)` bytes are read from the buffer
 * @param buffer Pointer to buffer which contains the encoded data
 * @param value Reference to a variable of type `T`, which will hold the decoded value
 * @return Number of bytes read from the buffer
 * @note `buffer` should have size atleast equal to `sizeof(T)`
 *
 * Example usage:
 * @code
 * // Assume buffer has a little endian encoded value
 * uint8_t buffer[4];
 * int x;
 * decode_le(buffer, x);
 * @endcode
 */
template <typename T> inline int decode_le(uint8_t *buffer, T &value)
{
    static_assert(std::is_integral<T>::value);
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    // Perform the decoding in unsigned type only, then convert it to the type of T
    using uT = std::make_unsigned_t<T>;
    uT val = 0;

    val |= static_cast<uT>(buffer[0]);
    if constexpr (sizeof(T) >= 2)
    {
        val |= static_cast<uT>(static_cast<uT>(buffer[1]) << 8);
    }
    if constexpr (sizeof(T) >= 4)
    {
        val |= static_cast<uT>(static_cast<uT>(buffer[2]) << 16);
        val |= static_cast<uT>(static_cast<uT>(buffer[3]) << 24);
    }
    if constexpr (sizeof(T) >= 8)
    {
        val |= static_cast<uT>(static_cast<uT>(buffer[4]) << 32);
        val |= static_cast<uT>(static_cast<uT>(buffer[5]) << 40);
        val |= static_cast<uT>(static_cast<uT>(buffer[6]) << 48);
        val |= static_cast<uT>(static_cast<uT>(buffer[7]) << 56);
    }
    value = static_cast<T>(val);
    return sizeof(T);
}

/**
 * @brief Decodes a big-endian encoded value from the buffer and stores it in `value`, the number of
 * bytes processed depends upon the type `T` of value passed
 * @tparam T Type of value to be decoded, `sizeof(T)` bytes are read from the buffer
 * @param buffer Pointer to buffer which contains the encoded data
 * @param value Reference to a variable of type `T`, which will hold the decoded value
 * @return Number of bytes read from the buffer
 * @note `buffer` should have size atleast equal to `sizeof(T)`
 *
 * Example usage:
 * @code
 * // Assume buffer has a big endian encoded value
 * uint8_t buffer[4];
 * int x;
 * decode_be(buffer, x);
 * @endcode
 */
template <typename T> inline int decode_be(uint8_t *buffer, T &value)
{
    static_assert(std::is_integral<T>::value);
    static_assert(sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);
    // Perform the decoding in unsigned type only, then convert it to the type of T
    using uT = std::make_unsigned_t<T>;
    uT val = 0;

    val |= static_cast<uT>(buffer[0]);
    if constexpr (sizeof(T) >= 2)
    {
        val <<= 8;
        val |= static_cast<uT>(buffer[1]);
    }
    if constexpr (sizeof(T) >= 4)
    {
        val <<= 8;
        val |= static_cast<uT>(buffer[2]);

        val <<= 8;
        val |= static_cast<uT>(buffer[3]);
    }
    if constexpr (sizeof(T) >= 8)
    {
        val <<= 8;
        val |= static_cast<uT>(buffer[4]);

        val <<= 8;
        val |= static_cast<uT>(buffer[5]);

        val <<= 8;
        val |= static_cast<uT>(buffer[6]);

        val <<= 8;
        val |= static_cast<uT>(buffer[7]);
    }
    value = static_cast<T>(val);
    return sizeof(T);
}

/**
 * @brief Encodes multiple values in little-endian format and then stores them in a buffer
 * @tparam T Type of the first parameter to encode
 * @tparam Args Type of additional values to pack into the buffer
 * @param buffer Pointer to the buffer where the encoded values will be written to
 * @param value The first value to encode
 * @param Args Additional values to pack
 * @return Number of bytes written to the buffer
 * @note `buffer` should be of sufficient length to prevent overflow, i.e. it should be atleast
 * equal to the sum of sizes of the types passed
 */
template <typename T, typename... Args> inline int encode_le(uint8_t *buffer, T value, Args... args)
{
    auto nbytes = encode_le(buffer, value);
    return nbytes + encode_le(buffer + sizeof(T), args...);
}

/**
 * @brief Encodes multiple values in big-endian format and then stores them in a buffer
 * @tparam T Type of the first parameter to encode
 * @tparam Args Type of additional values to pack into the buffer
 * @param buffer Pointer to the buffer where the encoded values will be written to
 * @param value The first value to encode
 * @param Args Additional values to pack
 * @return Number of bytes written to the buffer
 * @note `buffer` should be of sufficient length to prevent overflow, i.e. it should be atleast
 * equal to the sum of sizes of the types passed
 */
template <typename T, typename... Args> inline int encode_be(uint8_t *buffer, T value, Args... args)
{
    auto nbytes = encode_be(buffer, value);
    return nbytes + encode_be(buffer + sizeof(T), args...);
}

/**
 * @brief Decodes multiple values in little-endian format and stores the decoded value in the passed
 * references
 * @tparam T Type of the first parameter to decode
 * @tparam Args Type of additional values to unpack from the buffer
 * @param buffer Pointer to the buffer where the encoded values are present
 * @param value The first value to decode
 * @param Args Additional values to unpack
 * @return Number of bytes processed from the buffer
 * @note `buffer` should be of sufficient length to prevent overflow, i.e. it should be atleast
 * equal to the sum of sizes of the types passed
 */
template <typename T, typename... Args>
inline int decode_le(uint8_t *buffer, T &value, Args &...args)
{
    auto nbytes = decode_le(buffer, value);
    return nbytes + decode_le(buffer + sizeof(T), args...);
}

/**
 * @brief Decodes multiple values in big-endian format and stores the decoded value in the passed
 * references
 * @tparam T Type of the first parameter to decode
 * @tparam Args Type of additional values to unpack from the buffer
 * @param buffer Pointer to the buffer where the encoded values are present
 * @param value The first value to decode
 * @param Args Additional values to unpack, each value will be decoded from the appropriate offset
 * in the buffer
 * @return Number of bytes processed from the buffer
 * @note `buffer` should be of sufficient length to prevent overflow, i.e. it should be atleast
 * equal to the sum of sizes of the types passed
 */
template <typename T, typename... Args>
inline int decode_be(uint8_t *buffer, T &value, Args &...args)
{
    auto nbytes = decode_be(buffer, value);
    return nbytes + decode_be(buffer + sizeof(T), args...);
}

/**
 * @brief Encodes a value into a buffer with specified endianness.
 *
 * @tparam T The type of the value to encode. Must be an integral, float, or double type.
 * @tparam endianness The endianness to use for encoding. Must be either endian::little or
 * endian::big.
 * @param buffer The buffer to encode the value into.
 * @param value The value to encode.
 * @return int The number of bytes written to the buffer.
 *
 * @note buffer should be of sufficient length to hold the encoded value
 */
template <endian endianness, typename T> inline int encode(uint8_t *buffer, T value)
{

    if constexpr (std::is_integral<T>::value)
    {
        if constexpr (endianness == endian::little)
            return encode_le(buffer, value);
        else
            return encode_be(buffer, value);
    }
    else if constexpr (internal::is_float<T>::value)
    {
        return encode_float(buffer, value);
    }
    else if constexpr (internal::is_double<T>::value)
    {
        return encode_double(buffer, value);
    }
    else
    {
        static_assert(internal::False<T>{}, "Invalid type passed to encode, only"
                                            "integers and real numbers supported");
    }
}

/**
 * @brief Decodes a value into a buffer with specified endianness (for integral types).
 *
 * @tparam endianness The endianness to use for decoding. Must be either endian::little or
 * endian::big.
 * @tparam T The type of the value to decode to. Must be an integral, float, or double type.
 * @param buffer The buffer to decode the value from.
 * @param value Where to decoded value will be stored
 * @return int The number of bytes read from the buffer.
 *
 * @note buffer should be of sufficient length
 */
template <endian endianness, typename T> inline int decode(uint8_t *buffer, T &value)
{
    if constexpr (std::is_integral<T>::value)
    {
        if constexpr (endianness == endian::little)
            return decode_le(buffer, value);
        else
            return decode_be(buffer, value);
    }
    else if constexpr (internal::is_float<T>::value)
    {
        return decode_float(buffer, value);
    }
    else if constexpr (internal::is_double<T>::value)
    {
        return decode_double(buffer, value);
    }
    else
    {
        static_assert(internal::False<T>{}, "Invalid type passed to decode, only"
                                            "integers and real numbers supported");
    }
}

template <endian endianness, typename T, typename... Args>
inline int encode(uint8_t *buffer, T value, Args... args)
{
    auto nbytes = encode<endianness, T>(buffer, value);
    return nbytes + encode<endianness>(buffer + sizeof(T), args...);
}

template <endian endianness, typename T, typename... Args>
inline int decode(uint8_t *buffer, T &value, Args &...args)
{
    auto nbytes = decode<endianness, T>(buffer, value);
    return nbytes + decode<endianness>(buffer + sizeof(T), args...);
}

/**
 * @brief Encodes an array with a length prefix into a buffer.
 *
 * @tparam endianness The endianness to use for encoding
 * @tparam T The type of the elements in the array.
 * @tparam U The type used to encode the length of the array.
 * @param buffer The buffer where the encoded data will be stored.
 * @param arr The array of elements to encode.
 * @param n The number of elements in the array.
 * @return The total number of bytes written to the buffer.
 * @note buffer should be of size atleast equal to `sizeof(U) + sizeof(T) * n`
 */
template <endian endianness, typename T, typename U>
inline int encode_length_prefixed(uint8_t *buffer, T *arr, U n)
{
    encode<endianness, U>(buffer, n);

    buffer += sizeof(U);

    for (U i = 0; i < n; ++i)
    {
        encode<endianness, T>(buffer, arr[i]);
        buffer += sizeof(T);
    }
    return sizeof(U) + static_cast<int>(n * sizeof(T));
}

/**
 * @brief Decodes a length-prefixed array from a buffer.
 * @tparam endianness The endianness of the data
 * @tparam T The type of the elements in the array.
 * @tparam U The type used for the length prefix.
 * @param buffer The buffer containing the length-prefixed array to decode.
 * @param arr Pointer to the array where the decoded elements will be stored.
 * @param max_arr_length The maximum length of the array to prevent buffer overflow.
 * @return The total number of bytes read from the buffer, or -1 if the length read exceeds
 * max_arr_length
 */

template <endian endianness, typename T, typename U>
inline int decode_length_prefixed(uint8_t *buffer, T *arr, U max_arr_length)
{
    U arr_length;
    decode<endianness, U>(buffer, arr_length);

    if (arr_length > max_arr_length)
    {
        return -1;
    }

    buffer += sizeof(U);

    for (U i = 0; i < arr_length; ++i)
    {
        decode<endianness, T>(buffer, arr[i]);
        buffer += sizeof(T);
    }
    return sizeof(U) + static_cast<int>(arr_length * sizeof(T));
}

/**
 * @brief Encodes a string as a length prefixed array into a buffer
 * @tparam endianness Endianness of the length to be written
 * @param buffer The buffer where the encoded string will be stored
 * @param s The string to be encoded
 * @return The number of bytes written to the buffer,
 * @note `buffer` should be of size atleast equal to `sizeof(size_t) + s.size()`
 */
template <endian endianness>
inline int encode_length_prefixed(uint8_t *buffer, const std::string &s)
{
    return encode_length_prefixed<endianness>(buffer, s.data(), s.size());
}

/**
 * @brief Decodes a length-prefixed string from a buffer.
 * @tparam endianness Endianness of the length in the buffer
 * @param buffer The buffer containing the length-prefixed string to decode.
 * @param s Reference to a string into which the decoded value will be stored
 * @param max_string_length The maximum length of the string to prevent buffer overflow.
 * @return The total number of bytes read from the buffer, or -1 if the length read exceeds
 * max_string_length
 * @note This function creates a new string, then moves it into s, so it is fine if s is empty
 */
template <endian endianness>
inline int decode_length_prefixed(uint8_t *buffer, std::string &s, size_t max_string_length)
{
    std::string str;
    str.resize(max_string_length);
    auto bytes_read = decode_length_prefixed<endianness>(buffer, str.data(), max_string_length);
    if (bytes_read == -1)
    {
        return -1;
    }
    str.resize(bytes_read - sizeof(std::string::size_type));
    s = std::move(str);
    return bytes_read;
}

/**
 * @brief Encodes a vector as a length prefixed array into a buffer
 * @tparam endianness Endianness of the length in the buffer
 * @tparam T The type of elements of the vector
 * @param buffer The buffer where the encoded vector will be stored
 * @param v The vector to be encoded
 * @return The number of bytes written to the buffer,
 */
template <endian endianness, typename T>
inline int encode_length_prefixed(uint8_t *buffer, const std::vector<T> &v)
{
    return encode_length_prefixed<endianness>(buffer, v.data(), v.size());
}

/**
 * @brief Decodes a length-prefixed vector from a buffer.
 * @tparam endianness Endianness of the length in the buffer
 * @tparam T The type of elements of the vector
 * @param buffer The buffer containing the length-prefixed vector to decode.
 * @param v Reference to a vector into which the decoded value will be stored
 * @param max_length The maximum length of the vector to prevent buffer overflow.
 * @return The total number of bytes read from the buffer, or -1 if the length read exceeds
 * max_length
 * @note This function creates a new vector, then moves it into v, so it is fine if v is empty
 */
template <endian endianness, typename T>
inline int decode_length_prefixed(uint8_t *buffer, std::vector<T> &v, size_t max_length)
{
    std::vector<T> vec;
    vec.resize(max_length);
    auto bytes_read = decode_length_prefixed<endianness>(buffer, vec.data(), max_length);
    if (bytes_read == -1)
    {
        return -1;
    }
    vec.resize((bytes_read - sizeof(typename std::vector<T>::size_type)) / sizeof(T));
    v = std::move(vec);
    return bytes_read;
}
} // namespace bytes

/**
 * Functions in this namespace operate over streams - `istream` and `ostream`. They do not require
 * manual management of buffers
 */
namespace stream
{

template <endian endianness, typename T> inline std::ostream &write(std::ostream &os, T value)
{
    std::vector<uint8_t> buffer;
    // Allocate a buffer of necessary size
    if constexpr (std::is_integral<T>::value || std::is_floating_point<T>::value)
    {
        buffer.resize(sizeof(T));
        bytes::encode<endianness>(buffer.data(), value);
    }
    // If T is a const string literal
    else if constexpr (std::is_same<T, const char *>::value)
    {
        size_t str_size = strlen(value);
        buffer.resize(sizeof(size_t) + str_size);
        bytes::encode_length_prefixed<endianness>(buffer.data(), value, str_size);
    }
    // If T is a vector or a string, also allocate memory of sizeof(size_t) for the length
    else if constexpr (std::is_same<T, std::string>::value ||
                       std::is_same<T, std::vector<typename T::value_type>>::value)
    {
        buffer.resize(sizeof(size_t) + value.size() * sizeof(typename T::value_type));
        bytes::encode_length_prefixed<endianness>(buffer.data(), value);
    }
    else
    {
        static_assert(internal::False<T>{},
                      "Invalid type passed to write, can only encode integers, real "
                      "numbers, vectors, and strings");
    }
    return os.write(reinterpret_cast<const char *>(buffer.data()), buffer.size());
}

template <endian endianness, typename T>
inline std::istream &read(std::istream &is, T &value,
                          size_t max_elements = DEFAULT_MAX_NUMBER_OF_ELEMENTS)
{
    std::vector<uint8_t> buffer;
    // Allocate a buffer of necessary size
    if constexpr (std::is_integral<T>::value || std::is_floating_point<T>::value)
    {
        buffer.resize(sizeof(T));
        is.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (!is)
            return is;
        bytes::decode<endianness>(buffer.data(), value);
    }
    // If T is a vector or a string, also allocate memory of sizeof(size_t) for the length
    else if constexpr (std::is_same<T, std::string>::value ||
                       std::is_same<T, std::vector<typename T::value_type>>::value)
    {
        buffer.resize(sizeof(size_t));
        is.read(reinterpret_cast<char *>(buffer.data()), buffer.size());
        if (!is)
            return is;
        size_t sz = 0;
        if (bytes::decode<endianness>(buffer.data(), sz) != sizeof(size_t))
        {
            // Throw error
            throw std::runtime_error("Sequence size could not be determined");
        }
        if (sz > max_elements)
        {
            throw std::runtime_error("Data contains more elements than max_elements, read failed");
        }
        buffer.resize(sz * sizeof(typename T::value_type) + sizeof(size_t));
        is.read(reinterpret_cast<char *>(buffer.data() + sizeof(size_t)),
                buffer.size() - sizeof(size_t));
        if (!is)
            return is;

        bytes::decode_length_prefixed<endianness>(buffer.data(), value, max_elements);
    }
    else
    {
        static_assert(internal::False<T>{},
                      "Invalid type passed to read, can only decode integers, real "
                      "numbers, vectors, and strings");
    }
    return is;
}


} // namespace stream

namespace internal
{
// Code taken from https://beej.us/guide/bgnet/source/examples/ieee754.c
// Does not support NaN and infinity

/**
 * This function packs a double as an unsigned 64 bit integer, which can be written to a buffer
 */
template <unsigned bits, unsigned expbits> inline uint64_t pack754(long double f)
{
    long double fnorm;
    int shift;
    long long sign, exp, significand;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (f == 0.0)
        return 0; // get this special case out of the way

    // check sign and begin normalization
    if (f < 0)
    {
        sign = 1;
        fnorm = -f;
    }
    else
    {
        sign = 0;
        fnorm = f;
    }

    // get the normalized form of f and track the exponent
    shift = 0;
    while (fnorm >= 2.0)
    {
        fnorm /= 2.0;
        shift++;
    }
    while (fnorm < 1.0)
    {
        fnorm *= 2.0;
        shift--;
    }
    fnorm = fnorm - 1.0;

    // calculate the binary form (non-float) of the significand data
    significand =
        static_cast<long long>(fnorm * (static_cast<float>(1LL << significandbits) + 0.5f));

    // get the biased exponent
    exp = shift + ((1 << (expbits - 1)) - 1); // shift + bias

    // return the final answer
    return static_cast<uint64_t>((sign << (bits - 1)) | (exp << (bits - expbits - 1)) |
                                 significand);
}

/**
 * This function unpacks a unsigned 64 bit integer into a double
 */
template <unsigned bits, unsigned expbits> inline long double unpack754(uint64_t i)
{
    long double result;
    long long shift;
    unsigned bias;
    unsigned significandbits = bits - expbits - 1; // -1 for sign bit

    if (i == 0)
        return 0.0;

    // pull the significand
    result = (i & ((1LL << significandbits) - 1)); // mask
    result /= (1LL << significandbits);            // convert back to float
    result += 1.0f;                                // add the one back on

    // deal with the exponent
    bias = static_cast<unsigned int>((1 << (expbits - 1)) - 1);
    shift = static_cast<long long>(((i >> significandbits) & ((1LL << expbits) - 1)) - bias);
    while (shift > 0)
    {
        result *= 2.0;
        shift--;
    }
    while (shift < 0)
    {
        result /= 2.0;
        shift++;
    }

    // sign it
    result *= (i >> (bits - 1)) & 1 ? -1.0 : 1.0;

    return result;
}
} // namespace internal
} // namespace datapacker
#endif // A_DATAPACKER_H