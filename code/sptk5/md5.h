/* MD5
 converted to C++ class by Frank Thilo (thilo@unix-ag.org)
 for bzflag (http://www.bzflag.org)

   based on:

   md5.h and md5.c
   reference implementation of RFC 1321

   Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.

*/

#pragma once

#include "Buffer.h"
#include <sptk5/String.h>
#include <sptk5/sptk.h>

namespace sptk {
/**
 * Calculates MD5 hashes of strings or byte arrays.
 *
 * usage: 1) feed it blocks of uchars with update().
 *        2) finalize().
 *        3) get hexDigest() string
 *            or
 *        MD5(std::string).hexDigest().
 *
 * assumes that char is 8 bit and int is 32 bit
 */
class SP_EXPORT MD5
{
public:
    /**
     * @brief Defines the integer type.
     */
    using size_type = unsigned int; // must be 32bit

    /**
     * @brief Default constructor.
     */
    MD5();

    /**
     * @briefShortcut constructor.
     *
     * Immediately processes text.
     * The result can be read with hexDigest().
     * @param data          Text to MD5.
     */
    explicit MD5(const Buffer& data);

    /**
     * @brief Adds data portion to MD5.
     * @param buffer        Input data.
     * @param length        Size of input data.
     */
    void update(const unsigned char* buffer, size_t length);

    /**
     * @brief Adds data portion to MD5.
     * @param buffer        Input data.
     * @param length        Size of input data.
     */
    void update(const char* buffer, size_t length);

    /**
     * @brief Finalizes MD5 sum.
     */
    MD5& finalize();

    /**
     * @brief Returns hexadecimal presentation of MD5 sum.
     */
    [[nodiscard]] String hexDigest() const;

private:
    /**
     * @brief Initializes decoding state.
     */
    void init();

    /**
     * @brief Blocksize constant.
     */
    static constexpr auto blockSize = 64;

    /**
     * @brief Internal transformation.
     */
    void transform(const uint8_t* block);

    /**
     * @brief Internal decode.
     */
    static void decode(uint32_t* output, const uint8_t* input, size_type len);

    /**
     * @brief Internal encode.
     */
    static void encode(uint8_t* output, const uint32_t* input, size_type len);

    /**
     * @brief MD5 finalized flag.
     */
    bool finalized {false};

    /**
     * @brief bytes that didn't fit in the last 64-byte chunk.
     */
    std::array<uint8_t, blockSize> buffer {};

    /**
     * @brief 64bit counter for number of bits (lo, hi).
     */
    std::array<uint32_t, 2> count {};

    /**
     * @brief Digest so far.
     */
    std::array<uint32_t, 4> state {};

    /**
     * @brief The result.
     */
    std::array<uint8_t, 16> digest {};


    // Low-level logic operations
    static inline uint32_t F(uint32_t x, uint32_t y, uint32_t z);

    static inline uint32_t G(uint32_t x, uint32_t y, uint32_t z);

    static inline uint32_t H(uint32_t x, uint32_t y, uint32_t z);

    static inline uint32_t I(uint32_t x, uint32_t y, uint32_t z);

    static inline uint32_t rotate_left(uint32_t x, int n);

    static inline void FF(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);

    static inline void GG(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);

    static inline void HH(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);

    static inline void II(uint32_t& a, uint32_t b, uint32_t c, uint32_t d, uint32_t x, uint32_t s, uint32_t ac);
};

/**
 * @brief Single data-to-MD5 function.
 */
SP_EXPORT String md5(const Buffer& data);

/**
 * @brief Single data-to-MD5 function.
 */
SP_EXPORT String md5(const String& data);

} // namespace sptk
