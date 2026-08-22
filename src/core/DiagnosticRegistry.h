#pragma once

#include <cstddef>
#include <string>

#ifdef NDEBUG

// Release: no-op macros -- zero overhead
#define DIAG_REGISTER_MUTEX(name, ptr)
#define DIAG_UNREGISTER_MUTEX(name)
#define DIAG_REGISTER_PTR(name, ptr, refCount)
#define DIAG_UNREGISTER_PTR(name)
#define DIAG_UPDATE_REFCOUNT(name, refCount)

namespace embview::core
{
    struct DiagnosticRegistry
    {
        static DiagnosticRegistry& instance()
        {
            static DiagnosticRegistry s;
            return s;
        }
        void dumpToLog() {}
        std::string snapshot() { return ""; }
    };
} // namespace embview::core

#else

#include <map>
#include <shared_mutex>

namespace embview::core
{
    /**
     * @brief Debug-only registry tracking named mutexes and smart pointers.
     *
     * Provides introspection into active synchronization primitives and
     * reference-counted resources. Stripped entirely in Release builds.
     */
    class DiagnosticRegistry
    {
    public:

        /**
         * @brief Getter for the instance.
         * @return The Diagnositc Registry instance
         */
        static DiagnosticRegistry& instance();

        /**
         * @brief Registers a mutex
         * @param name the name of the mutex
         * @param ptr A void ptr
         */
        void registerMutex(std::string name, void* ptr);

        /**
         * @brief Unregisters a mutex
         * @param name the name of the mutex
         */
        void unregisterMutex(const std::string& name);

        /**
         * @brief Registers a pointer
         * @param name The name of the pointer
         * @param ptr The actual pointer
         * @param refCount The reference count
         */
        void registerPtr(std::string name, void* ptr, std::size_t refCount);

        /**
         * @brief Unregisters a pointer
         * @param name The name of the pointer
         */
        void unregisterPtr(const std::string& name);

        /**
         * @brief Updates the reference count
         * @param name The name of the reference
         * @param refCount The reference count
         */
        void updateRefCount(const std::string& name, std::size_t refCount);

        /**
         * @brief Get a formatted table of all tracked resources
         * @return The snapshot
         */
        std::string snapshot() const;

        /**
         * @brief Write the snapshot to spdlog at info level
         */
        void dumpToLog() const;

    private:

        /**
         * @brief Default constructor
         */
        DiagnosticRegistry() = default;

        /**
         * @brief Struct representing a mutex entry \struct MutexEntry
         */
        struct MutexEntry
        {
            void* ptr = nullptr;
        };

        /**
         * @brief Struct representing a pointer entry \struct PtrEntry
         */
        struct PtrEntry
        {
            void* ptr = nullptr;
            std::size_t refCount = 0;
        };

        mutable std::shared_mutex m_mutex;
        std::map<std::string, MutexEntry> m_mutexes;
        std::map<std::string, PtrEntry> m_ptrs;
    };
} // namespace embview::core

/**
 * @brief Helper macro to register a mutex
 * @param name The name of the mutex
 * @param ptr The ptr
 */
#define DIAG_REGISTER_MUTEX(name, ptr) \
    embview::core::DiagnosticRegistry::instance().registerMutex(name, ptr)

/**
 * @brief Helper macro to unregister a mutex
 * @param name The name of the mutex
 */
#define DIAG_UNREGISTER_MUTEX(name) \
    embview::core::DiagnosticRegistry::instance().unregisterMutex(name)

/**
 * @brief Helper macro to register a pointer
 * @param name The name of the pointer
 * @param ptr The actual pointer
 * @param refCount The reference count
 */
#define DIAG_REGISTER_PTR(name, ptr, refCount) \
    embview::core::DiagnosticRegistry::instance().registerPtr(name, ptr, refCount)

/**
 * @brief Helper macro to unregister a pointer
 * @param name The name of the pointer
 */
#define DIAG_UNREGISTER_PTR(name) \
    embview::core::DiagnosticRegistry::instance().unregisterPtr(name)

/**
 * @brief Helper macro to update the reference count of a pointer
 * @param name The mame of the reference
 * @param refCount The reference count
 */
#define DIAG_UPDATE_REFCOUNT(name, refCount) \
    embview::core::DiagnosticRegistry::instance().updateRefCount(name, refCount)

#endif
