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
        static DiagnosticRegistry& instance();

        void registerMutex(std::string name, void* ptr);
        void unregisterMutex(const std::string& name);

        void registerPtr(std::string name, void* ptr, std::size_t refCount);
        void unregisterPtr(const std::string& name);
        void updateRefCount(const std::string& name, std::size_t refCount);

        /// @brief Get a formatted table of all tracked resources.
        std::string snapshot() const;

        /// @brief Write the snapshot to spdlog at info level.
        void dumpToLog();

    private:
        DiagnosticRegistry() = default;

        struct MutexEntry
        {
            void* ptr = nullptr;
        };

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

#define DIAG_REGISTER_MUTEX(name, ptr) \
    embview::core::DiagnosticRegistry::instance().registerMutex(name, ptr)
#define DIAG_UNREGISTER_MUTEX(name) \
    embview::core::DiagnosticRegistry::instance().unregisterMutex(name)
#define DIAG_REGISTER_PTR(name, ptr, refCount) \
    embview::core::DiagnosticRegistry::instance().registerPtr(name, ptr, refCount)
#define DIAG_UNREGISTER_PTR(name) \
    embview::core::DiagnosticRegistry::instance().unregisterPtr(name)
#define DIAG_UPDATE_REFCOUNT(name, refCount) \
    embview::core::DiagnosticRegistry::instance().updateRefCount(name, refCount)

#endif
