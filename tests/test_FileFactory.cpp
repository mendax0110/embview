#include <gtest/gtest.h>

#include "core/FileFactory.h"

#include <filesystem>

using namespace embview::core;

namespace
{
    std::filesystem::path tempFilePath(const std::string& suffix)
    {
        return std::filesystem::temp_directory_path() / ("embview_filefactory_" + suffix);
    }
} // namespace

TEST(FileFactoryTest, BuiltinCreatorCreatesBlob)
{
    const auto data = std::vector<uint8_t>{'o', 'k'};
    const auto blob = FileFactory::instance().create(FileTypeId::txt, data);

    ASSERT_NE(blob, nullptr);
    EXPECT_EQ(blob->type(), FileTypeId::txt);
    EXPECT_EQ(blob->data(), data);
}

TEST(FileFactoryTest, SaveAndLoadRoundTrip)
{
    const auto path = tempFilePath("roundtrip.bin");
    const auto data = std::vector<uint8_t>{0xAA, 0xBB, 0xCC, 0xDD};

    const auto blob = FileFactory::instance().create(FileTypeId::binary, data);
    FileFactory::saveToFile(*blob, path);

    const auto loaded = FileFactory::instance().loadFromFile(path, FileTypeId::binary);
    ASSERT_NE(loaded, nullptr);
    EXPECT_EQ(loaded->data(), data);

    std::error_code ec;
    std::filesystem::remove(path, ec);
}

TEST(FileFactoryTest, UnknownTypeThrows)
{
    auto& factory = FileFactory::instance();
    EXPECT_THROW(factory.create(static_cast<FileTypeId>(999), {}), std::runtime_error);
}

