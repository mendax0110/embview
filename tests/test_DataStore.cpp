#include <gtest/gtest.h>

#include "core/DataStore.h"

#include <algorithm>
#include <thread>
#include <vector>

using namespace embview::core;

class DataStoreTest : public ::testing::Test
{
protected:
    DataStore store{100}; // small capacity for testing

    static DataFrame makeFrame(const uint8_t channel, const double value, const double timestamp = 0.0)
    {
        return DataFrame{channel, timestamp, value};
    }
};

TEST_F(DataStoreTest, PushAndGetSingleChannel)
{
    store.push(makeFrame(0, 1.0));
    store.push(makeFrame(0, 2.0));
    store.push(makeFrame(0, 3.0));

    const auto data = store.getChannel(0);
    ASSERT_EQ(data.size(), 3u);
    EXPECT_DOUBLE_EQ(data[0].value, 1.0);
    EXPECT_DOUBLE_EQ(data[1].value, 2.0);
    EXPECT_DOUBLE_EQ(data[2].value, 3.0);
}

TEST_F(DataStoreTest, MultipleChannels)
{
    store.push(makeFrame(0, 10.0));
    store.push(makeFrame(1, 20.0));
    store.push(makeFrame(2, 30.0));

    EXPECT_EQ(store.getChannel(0).size(), 1u);
    EXPECT_EQ(store.getChannel(1).size(), 1u);
    EXPECT_EQ(store.getChannel(2).size(), 1u);
    EXPECT_DOUBLE_EQ(store.getChannel(1)[0].value, 20.0);
}

TEST_F(DataStoreTest, CapacityLimit)
{
    for (int i = 0; i < 150; ++i)
    {
        store.push(makeFrame(0, static_cast<double>(i)));
    }

    const auto data = store.getChannel(0);
    ASSERT_EQ(data.size(), 100u);
    // Oldest samples should have been dropped
    EXPECT_DOUBLE_EQ(data[0].value, 50.0);
    EXPECT_DOUBLE_EQ(data[99].value, 149.0);
}

TEST_F(DataStoreTest, GetEmptyChannel)
{
    const auto data = store.getChannel(99);
    EXPECT_TRUE(data.empty());
}

TEST_F(DataStoreTest, ActiveChannels)
{
    store.push(makeFrame(3, 1.0));
    store.push(makeFrame(7, 2.0));
    store.push(makeFrame(1, 3.0));

    auto channels = store.getActiveChannels();
    std::ranges::sort(channels.begin(), channels.end());

    ASSERT_EQ(channels.size(), 3u);
    EXPECT_EQ(channels[0], 1);
    EXPECT_EQ(channels[1], 3);
    EXPECT_EQ(channels[2], 7);
}

TEST_F(DataStoreTest, ChannelSize)
{
    store.push(makeFrame(0, 1.0));
    store.push(makeFrame(0, 2.0));

    EXPECT_EQ(store.getChannelSize(0), 2u);
    EXPECT_EQ(store.getChannelSize(1), 0u);
}

TEST_F(DataStoreTest, ClearAll)
{
    store.push(makeFrame(0, 1.0));
    store.push(makeFrame(1, 2.0));

    store.clear();

    EXPECT_TRUE(store.getChannel(0).empty());
    EXPECT_TRUE(store.getChannel(1).empty());
    EXPECT_TRUE(store.getActiveChannels().empty());
}

TEST_F(DataStoreTest, ClearSingleChannel)
{
    store.push(makeFrame(0, 1.0));
    store.push(makeFrame(1, 2.0));

    store.clearChannel(0);

    EXPECT_TRUE(store.getChannel(0).empty());
    EXPECT_EQ(store.getChannel(1).size(), 1u);
}

TEST_F(DataStoreTest, ThreadSafety)
{
    constexpr int numThreads = 4;
    constexpr int framesPerThread = 1000;

    auto writer = [this](const uint8_t channel)
    {
        for (int i = 0; i < framesPerThread; ++i)
        {
            store.push(makeFrame(channel, static_cast<double>(i)));
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.emplace_back(writer, static_cast<uint8_t>(i));
    }

    for (auto& t : threads)
    {
        t.join();
    }

    for (int i = 0; i < numThreads; ++i)
    {
        auto data = store.getChannel(static_cast<uint8_t>(i));
        EXPECT_EQ(data.size(), 100u); // capped at capacity
    }
}
