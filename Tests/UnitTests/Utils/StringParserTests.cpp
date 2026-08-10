#include <cstdint>

#include <gtest/gtest.h>

#include <Utils/StringParser.h>

TEST(StringParserTest, ParsesMaximumUint16Value)
{
    StringParser parser{"65535"};
    std::uint16_t value{};

    EXPECT_TRUE(parser.parseInt(value));
    EXPECT_EQ(value, 65535);
}

TEST(StringParserTest, RejectsUint16Overflow)
{
    for (const auto* input : {"65536", "655360"}) {
        StringParser parser{input};
        std::uint16_t value{123};

        EXPECT_FALSE(parser.parseInt(value));
        EXPECT_EQ(value, 123);
    }
}

TEST(StringParserTest, PreservesUint8BoundaryBehavior)
{
    StringParser maxParser{"255"};
    std::uint8_t maxValue{};
    EXPECT_TRUE(maxParser.parseInt(maxValue));
    EXPECT_EQ(maxValue, 255);

    for (const auto* input : {"256", "2560"}) {
        StringParser overflowParser{input};
        std::uint8_t overflowValue{123};
        EXPECT_FALSE(overflowParser.parseInt(overflowValue));
        EXPECT_EQ(overflowValue, 123);
    }
}
