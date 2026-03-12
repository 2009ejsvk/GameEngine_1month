#pragma once

#include <string>

namespace RuntimeConfigRegistry
{
    struct FSourceRegistration
    {
        const wchar_t* Id = nullptr;
        std::wstring Path;
        float CheckIntervalSeconds = 0.5f;
        void (*ResetToDefaults)() = nullptr;
        bool (*LoadFromFile)(const std::wstring& Path) = nullptr;
        void (*OnReloaded)() = nullptr;
    };

    std::wstring BuildExeRelativePath(const wchar_t* FileName);

    bool RegisterSource(const FSourceRegistration& Registration);
    bool PollAll(float DeltaTime);
    bool PollSource(const wchar_t* Id, float DeltaTime);
    bool ForceReload(const wchar_t* Id);
    void Reset();

    unsigned long long GetGlobalGeneration();
    unsigned long long GetSourceGeneration(const wchar_t* Id);
}
