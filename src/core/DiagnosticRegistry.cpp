#ifndef NDEBUG

#include "core/DiagnosticRegistry.h"

#include <spdlog/spdlog.h>

#include <format>
#include <sstream>

using namespace embview::core;

DiagnosticRegistry& DiagnosticRegistry::instance()
{
    static DiagnosticRegistry registry;
    return registry;
}

void DiagnosticRegistry::registerMutex(std::string name, void* ptr)
{
    std::unique_lock lock(m_mutex);
    m_mutexes[std::move(name)] = MutexEntry{ptr};
}

void DiagnosticRegistry::unregisterMutex(const std::string& name)
{
    std::unique_lock lock(m_mutex);
    m_mutexes.erase(name);
}

void DiagnosticRegistry::registerPtr(std::string name, void* ptr, std::size_t refCount)
{
    std::unique_lock lock(m_mutex);
    m_ptrs[std::move(name)] = PtrEntry{ptr, refCount};
}

void DiagnosticRegistry::unregisterPtr(const std::string& name)
{
    std::unique_lock lock(m_mutex);
    m_ptrs.erase(name);
}

void DiagnosticRegistry::updateRefCount(const std::string& name, std::size_t refCount)
{
    std::unique_lock lock(m_mutex);
    auto it = m_ptrs.find(name);
    if (it != m_ptrs.end())
    {
        it->second.refCount = refCount;
    }
}

std::string DiagnosticRegistry::snapshot() const
{
    std::shared_lock lock(m_mutex);
    std::ostringstream os;

    os << "\n=== Diagnostic Registry Snapshot ===\n";

    if (!m_mutexes.empty())
    {
        os << std::format("{:<40s} {:<10s} {:<18s}\n", "Mutex Name", "Type", "Address");
        os << std::string(68, '-') << "\n";
        for (const auto& [name, entry] : m_mutexes)
        {
            os << std::format("{:<40s} {:<10s} {:#018x}\n",
                              name, "mutex", reinterpret_cast<uintptr_t>(entry.ptr));
        }
    }

    if (!m_ptrs.empty())
    {
        os << std::format("\n{:<40s} {:<10s} {:<18s} {:<10s}\n", "Pointer Name", "Type", "Address", "RefCount");
        os << std::string(78, '-') << "\n";
        for (const auto& [name, entry] : m_ptrs)
        {
            os << std::format("{:<40s} {:<10s} {:#018x} {:<10d}\n",
                              name, "shared", reinterpret_cast<uintptr_t>(entry.ptr), entry.refCount);
        }
    }

    if (m_mutexes.empty() && m_ptrs.empty())
    {
        os << "(no tracked resources)\n";
    }

    os << "=== End Snapshot ===\n";
    return os.str();
}

void DiagnosticRegistry::dumpToLog()
{
    spdlog::info("{}", snapshot());
}

#endif
