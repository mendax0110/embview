#pragma once

#include <filesystem>
#include <vector>
#include <map>
#include <string>
#include <functional>
#include <memory>
#include <unordered_map>
#include <uuid/uuid.h>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief File Type Id.
     * \enum FileTypeId
     */
    enum class FileTypeId
    {
        json,
        csv,
        binary,
        txt
    };

    /**
     * @brief Storage Container Blob.
     * \class StorageContainerBlob
     */
    class StorageContainerBlob
    {
    public:
        /**
         * @brief Create a blob for a file payload.
         * @param data Raw file content.
         * @param type File type metadata.
         */
        StorageContainerBlob(std::vector<uint8_t> data, const FileTypeId type)
            : m_data(std::move(data)), m_type(type)
        {
            uuid_generate(m_id);
        }

        /**
         * @brief Get the raw payload bytes.
         * @return The stored payload data.
         */
        [[nodiscard]] const std::vector<uint8_t>& data() const { return m_data; }

        /**
         * @brief Get the file type.
         * @return The blob type.
         */
        [[nodiscard]] FileTypeId type() const { return m_type; }

        /**
         * @brief Get the blob identifier.
         * @return The unique blob ID.
         */
        [[nodiscard]] const uuid_t& id() const { return m_id; }

    private:
        std::vector<uint8_t> m_data;
        FileTypeId m_type;
        uuid_t m_id{};
    };

    /**
     * @brief File Factory.
     * \class FileFactory
     */
    class FileFactory
    {
    public:
        using Creator = std::function<std::unique_ptr<StorageContainerBlob>(const std::vector<uint8_t>&)>;

        /**
         * @brief Instance.
         * @return The result.
         */
        static FileFactory& instance();

        /**
         * @brief Register Creator.
         * @param type
         * @param creator
         */
        void registerCreator(FileTypeId type, Creator creator);

        /**
         * @brief Create.
         * @param type
         * @param data
         * @return The result.
         */
        std::unique_ptr<StorageContainerBlob> create(FileTypeId type, const std::vector<uint8_t>& data) const;

        /**
         * @brief Get File Path.
         * @param blob
         * @return The value.
         */
        std::filesystem::path getFilePath(const StorageContainerBlob& blob) const;

        /**
         * @brief Attach a saved path to a blob.
         * @param blob Blob being tracked.
         * @param path On-disk file path.
         */
        void setFilePath(const StorageContainerBlob& blob, const std::filesystem::path& path);

        /**
         * @brief Save To File.
         */
        static void saveToFile(const StorageContainerBlob& blob, const std::filesystem::path& path);

        /**
         * @brief Load a stored blob from disk.
         * @param path Source file path.
         * @param type Expected blob type.
         * @return The loaded blob.
         */
        std::unique_ptr<StorageContainerBlob> loadFromFile(const std::filesystem::path& path, FileTypeId type) const;

    private:
        /**
         * @brief File Factory.
         * @return The result.
         */
        FileFactory();

        /**
         * @brief Blob Id String.
         * @param blob The blob
         * @return The resulting string.
         */
        [[nodiscard]] static std::string blobIdString(const StorageContainerBlob& blob);

        /**
         * @brief Gets extension for a given type.
         * @param type The type
         * @return A string representing the type
         */
        [[nodiscard]] static std::string extensionForType(FileTypeId type);

        std::map<FileTypeId, Creator> m_creators;
        std::unordered_map<std::string, std::filesystem::path> m_blobPaths;
        std::filesystem::path m_blobDir = std::filesystem::temp_directory_path() / "embview_blobs";
    };
}