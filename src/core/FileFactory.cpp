#include "core/FileFactory.h"

#include <fstream>
#include <stdexcept>
#include <spdlog/spdlog.h>

using namespace embview::core;

namespace
{
    std::string typeToString(const FileTypeId type)
    {
        switch (type)
        {
            case FileTypeId::json: return "json";
            case FileTypeId::csv: return "csv";
            case FileTypeId::binary: return "binary";
            case FileTypeId::txt: return "txt";
        }
        return "unknown";
    }
} // namespace

FileFactory& FileFactory::instance()
{
    static FileFactory factory;
    return factory;
}

FileFactory::FileFactory()
{
    std::error_code ec;
    std::filesystem::create_directories(m_blobDir, ec);
    if (ec)
    {
        spdlog::warn("Failed to create blob directory {}: {}", m_blobDir.string(), ec.message());
    }

    registerCreator(FileTypeId::json, [](const std::vector<uint8_t>& data)
    {
        return std::make_unique<StorageContainerBlob>(data, FileTypeId::json);
    });
    registerCreator(FileTypeId::csv, [](const std::vector<uint8_t>& data)
    {
        return std::make_unique<StorageContainerBlob>(data, FileTypeId::csv);
    });
    registerCreator(FileTypeId::binary, [](const std::vector<uint8_t>& data)
    {
        return std::make_unique<StorageContainerBlob>(data, FileTypeId::binary);
    });
    registerCreator(FileTypeId::txt, [](const std::vector<uint8_t>& data)
    {
        return std::make_unique<StorageContainerBlob>(data, FileTypeId::txt);
    });
}

void FileFactory::registerCreator(FileTypeId type, Creator creator)
{
    if (!creator)
    {
        throw std::runtime_error("Creator function cannot be null");
    }

    m_creators[type] = std::move(creator);
    spdlog::debug("Registered file creator for type {}", typeToString(type));
}

std::unique_ptr<StorageContainerBlob> FileFactory::create(FileTypeId type, const std::vector<uint8_t>& data) const
{
    const auto it = m_creators.find(type);
    if (it == m_creators.end())
    {
        throw std::runtime_error("Unknown file type: " + std::to_string(static_cast<int>(type)));
    }
    return it->second(data);
}

std::filesystem::path FileFactory::getFilePath(const StorageContainerBlob& blob) const
{
    const auto id = blobIdString(blob);
    const auto it = m_blobPaths.find(id);
    if (it != m_blobPaths.end())
    {
        return it->second;
    }

    return m_blobDir / (id + extensionForType(blob.type()));
}

void FileFactory::setFilePath(const StorageContainerBlob& blob, const std::filesystem::path& path)
{
    if (path.empty())
    {
        throw std::runtime_error("File path cannot be empty");
    }

    m_blobPaths[blobIdString(blob)] = path;
}

void FileFactory::saveToFile(const StorageContainerBlob& blob, const std::filesystem::path& path)
{
    if (blob.data().empty())
    {
        throw std::runtime_error("Blob data is empty, cannot save to file");
    }

    const auto parent = path.parent_path();
    if (!parent.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parent, ec);
        if (ec)
        {
            throw std::runtime_error("Failed to create output directory: " + parent.string());
        }
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for writing: " + path.string());
    }

    file.write(reinterpret_cast<const char*>(blob.data().data()), static_cast<std::streamsize>(blob.data().size()));
}

std::unique_ptr<StorageContainerBlob> FileFactory::loadFromFile(const std::filesystem::path& path, const FileTypeId type) const
{
    if (!std::filesystem::exists(path))
    {
        throw std::runtime_error("File does not exist: " + path.string());
    }

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file for reading: " + path.string());
    }

    const std::vector<uint8_t> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return create(type, data);
}

std::string FileFactory::blobIdString(const StorageContainerBlob& blob)
{
    char id[37] = {};
    uuid_unparse(blob.id(), id);
    return id;
}

std::string FileFactory::extensionForType(const FileTypeId type)
{
    switch (type)
    {
        case FileTypeId::json: return ".json";
        case FileTypeId::csv: return ".csv";
        case FileTypeId::binary: return ".bin";
        case FileTypeId::txt: return ".txt";
    }
    return ".blob";
}

