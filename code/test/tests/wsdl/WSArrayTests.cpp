#include "sptk5/wsdl/WSArray.h"
#include "sptk5/wsdl/WSBasicTypes.h"

#include <algorithm>
#include <gtest/gtest.h>

namespace sptk {

// Test basic construction and initialization
TEST(SPTK_WSArray, DefaultConstructor)
{
    const WSArray<WSInteger> array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_TRUE(array.empty());
}

TEST(SPTK_WSArray, CopyConstructor)
{
    WSArray<WSInteger> original;
    for (size_t i = 0; i < 10; ++i)
    {
        original.push_back(WSInteger(i));
    }

    WSArray copy(original);

    EXPECT_EQ(original.size(), copy.size());

    for (size_t i = 0; i < original.size(); ++i)
    {
        auto orig = static_cast<int64_t>(original[i]);
        auto cp = static_cast<int64_t>(copy[i]);
        EXPECT_EQ(orig, cp);
    }
}

TEST(SPTK_WSArray, MoveConstructor)
{
    WSArray<WSInteger> original;
    for (size_t i = 0; i < 10; ++i)
    {
        original.push_back(WSInteger(i));
    }

    WSArray moved(std::move(original));

    for (size_t i = 0; i < 10; ++i)
    {
        auto cp = static_cast<int64_t>(moved[i]);
        EXPECT_EQ(i, cp);
    }
}

// Test assignment operators
TEST(SPTK_WSArray, CopyAssignment)
{
    WSArray<WSInteger> original;
    for (size_t i = 0; i < 10; ++i)
    {
        original.push_back(WSInteger(i));
    }

    WSArray<WSInteger> copy;

    copy = original;

    EXPECT_EQ(original.size(), copy.size());

    for (size_t i = 0; i < original.size(); ++i)
    {
        auto orig = static_cast<int64_t>(original[i]);
        auto cp = static_cast<int64_t>(copy[i]);
        EXPECT_EQ(orig, cp);
    }
}

TEST(SPTK_WSArray, MoveAssignment)
{
    WSArray<WSInteger> original;
    for (size_t i = 0; i < 10; ++i)
    {
        original.push_back(WSInteger(i));
    }

    WSArray<WSInteger> copy;

    copy = std::move(original);

    EXPECT_EQ(10, copy.size());

    for (size_t i = 0; i < 10; ++i)
    {
        auto cp = static_cast<int64_t>(copy[i]);
        EXPECT_EQ(i, cp);
    }
}

// Test element access
TEST(SPTK_WSArray, SubscriptOperator)
{
    WSArray<WSInteger> array;
    array.push_back(1);
    array.push_back(2);
    array.push_back(3);

    EXPECT_EQ(static_cast<int32_t>(array[0]), 1);
    EXPECT_EQ(static_cast<int32_t>(array[1]), 2);
    EXPECT_EQ(static_cast<int32_t>(array[2]), 3);

    // Test const version
    const WSArray<WSInteger>& const_array = array;
    EXPECT_EQ(static_cast<int32_t>(const_array[0]), 1);
    EXPECT_EQ(static_cast<int32_t>(const_array[1]), 2);
    EXPECT_EQ(static_cast<int32_t>(const_array[2]), 3);
}

// Test modifiers
TEST(SPTK_WSArray, Clear)
{
    WSArray<WSInteger> array;
    array.push_back(1);
    array.push_back(2);
    array.push_back(3);
    EXPECT_FALSE(array.empty());

    array.clear();
    EXPECT_TRUE(array.empty());
    EXPECT_EQ(array.size(), 0);
}

TEST(SPTK_WSArray, Resize)
{
    WSArray<WSInteger> array;
    for (size_t i = 0; i < 5; ++i)
    {
        array.push_back(WSInteger(i));
    }

    // Resize larger
    array.resize(10);
    EXPECT_EQ(array.size(), 10);
    for (size_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(static_cast<int32_t>(array[i]), i);
    }
    for (size_t i = 5; i < 10; ++i)
    {
        EXPECT_EQ(static_cast<int32_t>(array[i]), 0);
    }

    // Resize smaller
    array.resize(3);
    EXPECT_EQ(array.size(), 3);
    for (size_t i = 0; i < 3; ++i)
    {
        EXPECT_EQ(static_cast<int32_t>(array[i]), i);
    }
}

// Test exception safety
TEST(SPTK_WSArray, ExceptionSafety)
{
    WSArray<WSInteger> array;

    // Reserve should provide a strong exception guarantee
    array.reserve(10);
    auto capacity = array.capacity();

    // If reserve fails with larger number, original should be unchanged
    try
    {
        // This might throw bad_alloc on a very large allocation
        array.reserve(SIZE_MAX);
    }
    catch (...)
    {
        // The original state should be preserved
        EXPECT_EQ(array.capacity(), capacity);
        EXPECT_EQ(array.size(), 0);
    }
}

} // namespace sptk
