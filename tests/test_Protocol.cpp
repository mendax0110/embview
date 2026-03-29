#include <gtest/gtest.h>

#include "core/Protocol.h"

using namespace embview::core;

class ProtocolTest : public ::testing::Test
{
protected:
    Protocol protocol;

    static std::vector<uint8_t> makeValidFrame(const uint8_t channel, const double value)
    {
        DataFrame frame{};
        frame.channel = channel;
        frame.timestamp = 0.0;
        frame.value = value;
        return Protocol::encode(frame);
    }
};

TEST_F(ProtocolTest, ParseValidFrame)
{
    auto encoded = makeValidFrame(1, 42.0);
    protocol.feedData(encoded);

    const auto result = protocol.parseNext();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->channel, 1);
    EXPECT_DOUBLE_EQ(result->value, 42.0);
}

TEST_F(ProtocolTest, ParseMultipleFrames)
{
    auto frame1 = makeValidFrame(0, 1.0);
    auto frame2 = makeValidFrame(1, 2.0);
    auto frame3 = makeValidFrame(2, 3.0);

    std::vector<uint8_t> combined;
    combined.insert(combined.end(), frame1.begin(), frame1.end());
    combined.insert(combined.end(), frame2.begin(), frame2.end());
    combined.insert(combined.end(), frame3.begin(), frame3.end());

    protocol.feedData(combined);

    auto r1 = protocol.parseNext();
    auto r2 = protocol.parseNext();
    auto r3 = protocol.parseNext();
    auto r4 = protocol.parseNext();

    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());
    ASSERT_TRUE(r3.has_value());
    ASSERT_FALSE(r4.has_value());

    EXPECT_EQ(r1->channel, 0);
    EXPECT_EQ(r2->channel, 1);
    EXPECT_EQ(r3->channel, 2);

    EXPECT_DOUBLE_EQ(r1->value, 1.0);
    EXPECT_DOUBLE_EQ(r2->value, 2.0);
    EXPECT_DOUBLE_EQ(r3->value, 3.0);
}

TEST_F(ProtocolTest, SkipsGarbageBeforeValidFrame)
{
    std::vector<uint8_t> garbage = {0xFF, 0x00, 0x12, 0x34};
    auto validFrame = makeValidFrame(5, 99.9);

    std::vector<uint8_t> data;
    data.insert(data.end(), garbage.begin(), garbage.end());
    data.insert(data.end(), validFrame.begin(), validFrame.end());

    protocol.feedData(data);

    const auto result = protocol.parseNext();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->channel, 5);
    EXPECT_DOUBLE_EQ(result->value, 99.9);
}

TEST_F(ProtocolTest, RejectsCorruptedCRC)
{
    auto encoded = makeValidFrame(1, 42.0);
    // Corrupt the CRC byte (last byte)
    encoded.back() ^= 0xFF;

    protocol.feedData(encoded);

    const auto result = protocol.parseNext();
    EXPECT_FALSE(result.has_value());
}

TEST_F(ProtocolTest, RejectsInvalidLength)
{
    auto encoded = makeValidFrame(1, 42.0);
    // Corrupt the length byte
    encoded[1] = 0xFF;

    protocol.feedData(encoded);

    const auto result = protocol.parseNext();
    EXPECT_FALSE(result.has_value());
}

TEST_F(ProtocolTest, PartialFrameWaitsForMore)
{
    auto encoded = makeValidFrame(1, 42.0);

    // Feed only first half
    std::vector<uint8_t> firstHalf(encoded.begin(), encoded.begin() + 6);
    protocol.feedData(firstHalf);

    auto result = protocol.parseNext();
    EXPECT_FALSE(result.has_value());

    // Feed the rest
    std::vector<uint8_t> secondHalf(encoded.begin() + 6, encoded.end());
    protocol.feedData(secondHalf);

    result = protocol.parseNext();
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->channel, 1);
    EXPECT_DOUBLE_EQ(result->value, 42.0);
}

TEST_F(ProtocolTest, CRC8Deterministic)
{
    std::vector<uint8_t> data = {0x01, 0x02, 0x03};
    const uint8_t crc1 = Protocol::crc8(data);
    const uint8_t crc2 = Protocol::crc8(data);
    EXPECT_EQ(crc1, crc2);
}

TEST_F(ProtocolTest, CRC8EmptyInput)
{
    std::vector<uint8_t> empty;
    const uint8_t crc = Protocol::crc8(empty);
    EXPECT_EQ(crc, 0x00);
}

TEST_F(ProtocolTest, EncodeDecodeRoundtrip)
{
    DataFrame original{};
    original.channel = 7;
    original.timestamp = 0.0;
    original.value = -123.456;

    auto encoded = Protocol::encode(original);
    EXPECT_EQ(encoded.size(), Protocol::FRAME_SIZE);

    protocol.feedData(encoded);
    const auto decoded = protocol.parseNext();

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->channel, original.channel);
    EXPECT_DOUBLE_EQ(decoded->value, original.value);
}

TEST_F(ProtocolTest, TimestampIncrementsPerFrame)
{
    auto f1 = makeValidFrame(0, 1.0);
    auto f2 = makeValidFrame(0, 2.0);

    protocol.feedData(f1);
    protocol.feedData(f2);

    const auto r1 = protocol.parseNext();
    const auto r2 = protocol.parseNext();

    ASSERT_TRUE(r1.has_value());
    ASSERT_TRUE(r2.has_value());
    EXPECT_LT(r1->timestamp, r2->timestamp);
}
