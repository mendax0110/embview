#include <gtest/gtest.h>

#include "core/TransportFactory.h"

#include <nlohmann/json.hpp>

using namespace embview::core;

TEST(TransportFactoryTest, HasSerialType)
{
    const auto& factory = TransportFactory::instance();
    EXPECT_TRUE(factory.hasType("serial"));
}

TEST(TransportFactoryTest, UnknownTypeThrows)
{
    const auto& factory = TransportFactory::instance();
    const nlohmann::json config;
    EXPECT_THROW(factory.create("nonexistent", config), std::runtime_error);
}

TEST(TransportFactoryTest, CreateSerialReturnsNonNull)
{
    const auto& factory = TransportFactory::instance();
    nlohmann::json config;
    config["port"] = "COM_INVALID_TEST";
    config["baud"] = 9600;

    const auto transport = factory.create("serial", config);
    EXPECT_NE(transport, nullptr);
}

TEST(TransportFactoryTest, RegisterCustomType)
{
    auto& factory = TransportFactory::instance();

    factory.registerCreator("test_dummy", [](const nlohmann::json&) -> std::unique_ptr<ITransport>
    {
        return nullptr;
    });

    EXPECT_TRUE(factory.hasType("test_dummy"));
}
