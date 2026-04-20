#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <uuid/uuid.h>

namespace embview::core
{
    enum class FileTypeId
    {
        json,
        csv,
        binary,
        txt
    };

    class StorageContainerBlob
    {
    public:
        StorageContainerBlob(std::vector<uint8_t> data, const FileTypeId type)
            : m_data(std::move(data)), m_type(type)
        {
            uuid_generate(m_id);
        }

        [[nodiscard]] const std::vector<uint8_t>& data() const { return m_data; }
        [[nodiscard]] FileTypeId type() const { return m_type; }
        [[nodiscard]] const uuid_t& id() const { return m_id; }

    private:
        std::vector<uint8_t> m_data;
        FileTypeId m_type;
        uuid_t m_id;
    };

    class FileFactory
    {
    public:
        using Creator = std::function<std::unique_ptr<StorageContainerBlob>(const std::vector<uint8_t>&)>;

        static FileFactory& instance();

        void registerCreator(FileTypeId type, Creator creator);

        std::unique_ptr<StorageContainerBlob> create(FileTypeId type, const std::vector<uint8_t>& data) const;

        std::filesystem::path getFilePath(const StorageContainerBlob& blob) const;
        void setFilePath(const StorageContainerBlob& blob, const std::filesystem::path& path);

        static void saveToFile(const StorageContainerBlob& blob, const std::filesystem::path& path);
        std::unique_ptr<StorageContainerBlob> loadFromFile(const std::filesystem::path& path, FileTypeId type) const;

    private:
        FileFactory();

        [[nodiscard]] static std::string blobIdString(const StorageContainerBlob& blob);
        [[nodiscard]] static std::string extensionForType(FileTypeId type);

        std::map<FileTypeId, Creator> m_creators;
        std::unordered_map<std::string, std::filesystem::path> m_blobPaths;
        std::filesystem::path m_blobDir = std::filesystem::temp_directory_path() / "embview_blobs";
    };
}