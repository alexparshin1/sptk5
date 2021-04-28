/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Exception.h>
#include <sptk5/Brotli.h>

#if USE_GTEST
#include <sptk5/Base64.h>
#include <sptk5/StopWatch.h>
#include <sptk5/cutils>
#endif

#include <brotli/decode.h>
#include <brotli/encode.h>
#include <sptk5/ReadBuffer.h>

using namespace std;
using namespace sptk;

constexpr int DEFAULT_LGWIN = 24;
constexpr int BROTLI_WINDOW_GAP = 16;
#define BROTLI_MAX_BACKWARD_LIMIT(W) (((size_t)1 << (W)) - BROTLI_WINDOW_GAP)

static const size_t kBufferSize = 1 << 16;

class Context
{
public:
    Context(ReadBuffer& inputBuffer, Buffer& outputBuffer)
    : inputData(inputBuffer), outputData(outputBuffer), input_file_length(inputBuffer.length())
    {
        available_out = kBufferSize;
        next_out = output;
    }

    Context(const Context&) = delete;
    Context(Context&&) noexcept = delete;
    Context& operator = (const Context&) = delete;
    Context& operator = (Context&&) noexcept = delete;

    ~Context()
    {
        delete[] buffer;
    }

    BrotliEncoderState* createEncoderInstance();
    void CompressFile(BrotliEncoderState* s);
    void DecompressFile(BrotliDecoderState* s);

private:
    /* Parameters */
    int quality = 9;

    uint8_t* buffer = new uint8_t[kBufferSize * 2];

    uint8_t* input = buffer;
    uint8_t* output = buffer + kBufferSize;
    ReadBuffer& inputData;

    Buffer& outputData;
    int64_t input_file_length {0};  /* -1, if impossible to calculate */
    size_t available_in {0};
    const uint8_t* next_in = nullptr;

    size_t available_out;

    uint8_t* next_out = nullptr;

    BROTLI_BOOL HasMoreInput() const
    {
        return inputData.eof() ? BROTLI_FALSE : BROTLI_TRUE;
    }

    void ProvideInput()
    {
        available_in = inputData.available();
        if (available_in > kBufferSize)
            available_in = kBufferSize;
        inputData.read(input, available_in);
        next_in = input;
    }

    void WriteOutput()
    {
        auto out_size = (size_t) (next_out - output);
        if (out_size > 0)
            outputData.append((const char*) output, out_size);
    }

    void ProvideOutput()
    {
        WriteOutput();
        available_out = kBufferSize;
        next_out = output;
    }

    void FlushOutput()
    {
        WriteOutput();
        available_out = 0;
    }
};

BrotliEncoderState* Context::createEncoderInstance()
{
    auto* instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
    BrotliEncoderSetParameter(instance, BROTLI_PARAM_QUALITY, (uint32_t) quality);

    /* 0, or not specified by user; could be chosen by compressor. */
    uint32_t _lgwin = DEFAULT_LGWIN;
    /* Use file size to limit lgwin. */
    if (input_file_length >= 0) {
        _lgwin = BROTLI_MIN_WINDOW_BITS;
        while (BROTLI_MAX_BACKWARD_LIMIT(_lgwin) < (uint64_t) input_file_length) {
            ++_lgwin;
            if (_lgwin == BROTLI_MAX_WINDOW_BITS) break;
        }
    }
    BrotliEncoderSetParameter(instance, BROTLI_PARAM_LGWIN, _lgwin);

    if (input_file_length > 0) {
        uint32_t size_hint = input_file_length < (1 << 30) ?
                             (uint32_t)input_file_length : (1U << 30);
        BrotliEncoderSetParameter(instance, BROTLI_PARAM_SIZE_HINT, size_hint);
    }

    return instance;
}

void Context::CompressFile(BrotliEncoderState* s)
{
    BROTLI_BOOL is_eof = BROTLI_FALSE;
    for (;;) {
        if (available_in == 0 && !is_eof) {
            ProvideInput();
            is_eof = !HasMoreInput();
        }

        if (!BrotliEncoderCompressStream(s,
                                         is_eof ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS,
                                         &available_in, &next_in,
                                         &available_out, &next_out, nullptr)) {
            throw Exception("failed to compress data");
        }

        if (available_out == 0)
            ProvideOutput();

        if (BrotliEncoderIsFinished(s)) {
            FlushOutput();
            return;
        }
    }
}

void Context::DecompressFile(BrotliDecoderState* s)
{
    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
    for (;;) {
        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT) {
            if (!HasMoreInput())
                throw Exception("corrupt input data");
            ProvideInput();
        } else if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT) {
            ProvideOutput();
        } else if (result == BROTLI_DECODER_RESULT_SUCCESS) {
            FlushOutput();
            if (available_in != 0 || HasMoreInput())
                throw Exception("corrupt input data");
            return;
        } else {
            throw Exception("corrupt input data");
        }

        result = BrotliDecoderDecompressStream(s, &available_in,
                                               &next_in, &available_out, &next_out, nullptr);
    }
}

void Brotli::compress(Buffer& dest, const Buffer& src)
{
    ReadBuffer input(src.data(), src.bytes());
    auto context = make_shared<Context>(input, dest);

    auto* s = context->createEncoderInstance();

    context->CompressFile(s);
    BrotliEncoderDestroyInstance(s);
}

void Brotli::decompress(Buffer& dest, const Buffer& src)
{
    ReadBuffer input(src.data(), src.bytes());
    auto context = make_shared<Context>(input, dest);

    BrotliDecoderState* s = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!s)
        throw Exception("out of memory");

    BrotliDecoderSetParameter(s, BROTLI_DECODER_PARAM_LARGE_WINDOW, 1U);
    try {
        context->DecompressFile(s);
        BrotliDecoderDestroyInstance(s);
    }
    catch (const Exception&) {
        BrotliDecoderDestroyInstance(s);
        throw;
    }
}

#if USE_GTEST

static const String originalTestString = "This is a test of compression using Brotli algorithm";

#ifdef _WIN32
	static const String originalTestStringBase64 = "H4sIAAAAAAAACwvJyCxWAKJEhZLU4hKF/DSF5PzcgqLU4uLM/DyF0uLMvHQF96jMAoXEnPT8osySjFwAes7C0zIAAAA=";
#else
	static const String originalTestStringBase64 = "oZgBACBuY+u6dus1GkIllLABJwCJBp6OOGyMnS2IO2hX4F/C9cYbegYltUixcXITXgA=";
#endif

TEST(SPTK_Brotli, compress)
{
    Buffer compressed;
    String compressedBase64;
    Brotli::compress(compressed, Buffer(originalTestString));
    Base64::encode(compressedBase64, compressed);
    
    EXPECT_STREQ(originalTestStringBase64.c_str(), compressedBase64.c_str());
}

TEST(SPTK_Brotli, decompress)
{
    Buffer compressed;
    Buffer decompressed;

    Base64::decode(compressed, originalTestStringBase64);
    Brotli::decompress(decompressed, compressed);

    EXPECT_STREQ(originalTestString.c_str(), decompressed.c_str());
}

TEST(SPTK_Brotli, performance)
{
    Buffer data;
    Buffer compressed;
    Buffer decompressed;

    // Using uncompressed mplayer manual as test data
    data.loadFromFile(TEST_DIRECTORY "/data/mplayer.1");
    EXPECT_EQ(data.bytes(), size_t(345517));

    StopWatch stopWatch;
    stopWatch.start();
    Brotli::compress(compressed, data);
    stopWatch.stop();

    COUT("Brotli compressor:" << endl)
    COUT("Compressed " << data.bytes() << " bytes to " << compressed.bytes() << " bytes for "
        << stopWatch.seconds() << " seconds (" << data.bytes() / stopWatch.seconds() / 1E6 << " Mb/s)" << endl)

    stopWatch.start();
    Brotli::decompress(decompressed, compressed);
    stopWatch.stop();

    COUT("Decompressed " << compressed.bytes() << " bytes to " << decompressed.bytes() << " bytes for "
                       << stopWatch.seconds() << " seconds (" << decompressed.bytes() / stopWatch.seconds() / 1E6 << " Mb/s)" << endl)

    EXPECT_STREQ(data.c_str(), decompressed.c_str());
}

#endif
