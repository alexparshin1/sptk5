/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <brotli/decode.h>
#include <brotli/encode.h>
#include <sptk5/Brotli.h>
#include <sptk5/ReadBuffer.h>

using namespace std;
using namespace sptk;

constexpr int DEFAULT_LGWIN = 24;
constexpr int BROTLI_WINDOW_GAP = 16;

static size_t BROTLI_MAX_BACKWARD_LIMIT(uint32_t W)
{
    return (1U << W) - BROTLI_WINDOW_GAP;
}

static constexpr size_t kBufferSize = 1 << 16;

class Context
{
public:
    Context(ReadBuffer& inputBuffer, Buffer& outputBuffer)
        : inputData(inputBuffer)
        , outputData(outputBuffer)
        , input_file_length((int64_t) inputBuffer.size())
        , next_out(output)
    {
    }

    [[nodiscard]] BrotliEncoderState* createEncoderInstance() const;

    void CompressFile(BrotliEncoderState* state);

    void DecompressFile(BrotliDecoderState* state);

private:
    /* Parameters */
    static constexpr int highQuality = 9;
    int quality = highQuality;

    array<uint8_t, kBufferSize * 2> buffer {};

    uint8_t* input = buffer.data();
    uint8_t* output = buffer.data() + kBufferSize;
    ReadBuffer& inputData;

    Buffer& outputData;
    int64_t input_file_length {0}; /* -1, if impossible to calculate */
    size_t available_in {0};
    const uint8_t* next_in {nullptr};

    size_t available_out {kBufferSize};

    uint8_t* next_out {nullptr};

    [[nodiscard]] BROTLI_BOOL HasMoreInput() const
    {
        return inputData.eof() ? BROTLI_FALSE : BROTLI_TRUE;
    }

    void ProvideInput()
    {
        available_in = inputData.available();
        if (available_in > kBufferSize)
        {
            available_in = kBufferSize;
        }
        inputData.read(input, available_in);
        next_in = input;
    }

    void WriteOutput()
    {
        auto out_size = (size_t) (next_out - output);
        if (out_size > 0)
        {
            outputData.append(output, out_size);
        }
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

BrotliEncoderState* Context::createEncoderInstance() const
{
    auto* instance = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
    BrotliEncoderSetParameter(instance, BROTLI_PARAM_QUALITY, (uint32_t) quality);

    /* 0, or not specified by user; could be chosen by compressor. */
    uint32_t _lgwin = DEFAULT_LGWIN;
    /* Use file size to limit lgwin. */
    if (input_file_length >= 0)
    {
        _lgwin = BROTLI_MIN_WINDOW_BITS;
        while (BROTLI_MAX_BACKWARD_LIMIT(_lgwin) < (uint64_t) input_file_length)
        {
            ++_lgwin;
            if (_lgwin == BROTLI_MAX_WINDOW_BITS)
            {
                break;
            }
        }
    }
    BrotliEncoderSetParameter(instance, BROTLI_PARAM_LGWIN, _lgwin);

    if (input_file_length > 0)
    {
        constexpr uint32_t maxHintSize = uint32_t(-1) / 2;
        const uint32_t size_hint = input_file_length < maxHintSize ? (uint32_t) input_file_length : maxHintSize;
        BrotliEncoderSetParameter(instance, BROTLI_PARAM_SIZE_HINT, size_hint);
    }

    return instance;
}

void Context::CompressFile(BrotliEncoderState* state)
{
    BROTLI_BOOL is_eof = BROTLI_FALSE;
    for (;;)
    {
        if (available_in == 0 && !is_eof)
        {
            ProvideInput();
            is_eof = !HasMoreInput();
        }

        if (!BrotliEncoderCompressStream(state,
                                         is_eof ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS,
                                         &available_in, &next_in,
                                         &available_out, &next_out, nullptr))
        {
            throw Exception("failed to compress data");
        }

        if (available_out == 0)
        {
            ProvideOutput();
        }

        if (BrotliEncoderIsFinished(state))
        {
            FlushOutput();
            return;
        }
    }
}

void Context::DecompressFile(BrotliDecoderState* state)
{
    BrotliDecoderResult result = BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT;
    for (;;)
    {
        if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_INPUT)
        {
            if (!HasMoreInput())
            {
                throw Exception("corrupt input data");
            }
            ProvideInput();
        }
        else if (result == BROTLI_DECODER_RESULT_NEEDS_MORE_OUTPUT)
        {
            ProvideOutput();
        }
        else if (result == BROTLI_DECODER_RESULT_SUCCESS)
        {
            FlushOutput();
            if (available_in != 0 || HasMoreInput())
            {
                throw Exception("corrupt input data");
            }
            return;
        }
        else
        {
            throw Exception("corrupt input data");
        }

        result = BrotliDecoderDecompressStream(state, &available_in,
                                               &next_in, &available_out, &next_out, nullptr);
    }
}

void Brotli::compress(Buffer& dest, const Buffer& src)
{
    ReadBuffer input(src.data(), src.bytes());
    auto context = make_shared<Context>(input, dest);

    auto* state = context->createEncoderInstance();

    context->CompressFile(state);
    BrotliEncoderDestroyInstance(state);
}

void Brotli::decompress(Buffer& dest, const Buffer& src)
{
    ReadBuffer input(src.data(), src.bytes());
    auto context = make_shared<Context>(input, dest);

    BrotliDecoderState* state = BrotliDecoderCreateInstance(nullptr, nullptr, nullptr);
    if (!state)
    {
        throw Exception("out of memory");
    }

    BrotliDecoderSetParameter(state, BROTLI_DECODER_PARAM_LARGE_WINDOW, 1U);
    try
    {
        context->DecompressFile(state);
        BrotliDecoderDestroyInstance(state);
    }
    catch (const Exception&)
    {
        BrotliDecoderDestroyInstance(state);
        throw;
    }
}
