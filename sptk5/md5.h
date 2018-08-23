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

#ifndef BZF_MD5_H
#define BZF_MD5_H

#include <sptk5/sptk.h>
#include <sptk5/String.h>
#include <iostream>
#include "Buffer.h"

namespace sptk {

    /**
     * Calculates MD5 hashes of strings or byte arrays
     *
     * usage: 1) feed it blocks of uchars with update()
     *        2) finalize()
     *        3) get hexdigest() string
     *            or
     *        MD5(std::string).hexdigest()
     *
     * assumes that char is 8 bit and int is 32 bit
     */
    class MD5
    {
    public:
        /**
         * Defines integer type
         */
        typedef unsigned int size_type; // must be 32bit

        /**
         * Default constructor
         */
        MD5();

        /**
         * Shortcut constructor
         *
         * Immediately processes text.
         * The result can be read with hexdigest().
         * @param data          Text to MD5
         */
        explicit MD5(const Buffer& data);

        /**
         * Adds data portion to MD5
         * @param buffer        Input data
         * @param length        Size of input data
         */
        void update(const unsigned char* buffer, size_type length);

        /**
         * Adds data portion to MD5
         * @param buffer        Input data
         * @param length        Size of input data
         */
        void update(const char* buffer, size_type length);

        /**
         * Finalizes MD5 sum
         */
        MD5& finalize();

        /**
         * Returns hexadecimal presentation of MD5 sum
         */
        String hexdigest() const;

        /**
         * Prints MD5 sum to stream
         */
        friend std::ostream& operator<<(std::ostream&, MD5 md5);

    private:

        /**
         * Initializes decoding state
         */
        void init();

        /**
         * 8bit unsigned integer
         */
        typedef uint8_t uint1;

        /**
         * 32bit unsigned integer
         */
        typedef uint32_t uint4;

        /**
         * Blocksize const
         */
        enum
        {
            blocksize = 64
        };


        /**
         * Internal transformation
         */
        void transform(const uint1 block[blocksize]);

        /**
         * Internal decode
         */
        static void decode(uint4 output[], const uint1 input[], size_type len);

        /**
         * Internal encode
         */
        static void encode(uint1 output[], const uint4 input[], size_type len);

        /**
         * MD5 finalized flag
         */
        bool finalized;

        /**
         * bytes that didn't fit in last 64 byte chunk
         */
        uint1 buffer[blocksize];

        /**
         * 64bit counter for number of bits (lo, hi)
         */
        uint4 count[2];

        /**
         * digest so far
         */
        uint4 state[4];

        /**
         * the result
         */
        uint1 digest[16];


        // low level logic operations
        static inline uint4 F(uint4 x, uint4 y, uint4 z);

        static inline uint4 G(uint4 x, uint4 y, uint4 z);

        static inline uint4 H(uint4 x, uint4 y, uint4 z);

        static inline uint4 I(uint4 x, uint4 y, uint4 z);

        static inline uint4 rotate_left(uint4 x, int n);

        static inline void FF(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);

        static inline void GG(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);

        static inline void HH(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);

        static inline void II(uint4& a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
    };

    /**
     * Single data-to-MD5 function
     */
    String md5(const Buffer& data);

    /**
     * Single data-to-MD5 function
     */
    String md5(const String& data);

}

#endif
