#include "RuntimeConfigRegistry.h"
#include <Windows.h>
#include <sys/stat.h>
#include <algorithm>
#include <vector>

namespace
{
    struct FSourceState
    {
        std::wstring Id;
        std::wstring Path;
        float CheckIntervalSeconds = 0.5f;
        float CheckCooldown = 0.f;
        void (*ResetToDefaults)() = nullptr;
        bool (*LoadFromFile)(const std::wstring& Path) = nullptr;
        void (*OnReloaded)() = nullptr;
        __time64_t LastWriteTime = 0;
        bool HadFileOnLastLoad = false;
        bool Initialized = false;
        unsigned long long Generation = 0;
    };

    std::vector<FSourceState> GSources;
    unsigned long long GGlobalGeneration = 0;

    FSourceState* FindSource(const wchar_t* Id)
    {
        if (!Id || !*Id)
            return nullptr;

        const auto It = std::find_if(
            GSources.begin(),
            GSources.end(),
            [&](const FSourceState& Source)
            {
                return Source.Id == Id;
            });

        return It != GSources.end() ? &(*It) : nullptr;
    }

    bool TryGetFileWriteTime(
        const std::wstring& Path,
        __time64_t& OutWriteTime)
    {
        OutWriteTime = 0;

        struct _stat64 Stat = {};

        if (_wstat64(Path.c_str(), &Stat) != 0)
            return false;

        OutWriteTime = Stat.st_mtime;
        return true;
    }

    void ApplySource(
        FSourceState& Source,
        bool FileExists,
        __time64_t WriteTime)
    {
        if (Source.ResetToDefaults)
            Source.ResetToDefaults();

        if (FileExists && Source.LoadFromFile)
            Source.LoadFromFile(Source.Path);

        Source.LastWriteTime = FileExists ? WriteTime : 0;
        Source.HadFileOnLastLoad = FileExists;
        Source.Initialized = true;
        Source.CheckCooldown = Source.CheckIntervalSeconds;
        ++Source.Generation;
        ++GGlobalGeneration;

        if (Source.OnReloaded)
            Source.OnReloaded();
    }

    void EnsureInitialized(FSourceState& Source)
    {
        if (Source.Initialized)
            return;

        __time64_t WriteTime = 0;
        const bool FileExists = TryGetFileWriteTime(Source.Path, WriteTime);
        ApplySource(Source, FileExists, WriteTime);
    }

    bool PollSourceInternal(FSourceState& Source, float DeltaTime)
    {
        EnsureInitialized(Source);

        Source.CheckCooldown -= (std::max)(0.f, DeltaTime);

        if (Source.CheckCooldown > 0.f)
            return false;

        Source.CheckCooldown = Source.CheckIntervalSeconds;

        __time64_t WriteTime = 0;
        const bool FileExists = TryGetFileWriteTime(Source.Path, WriteTime);

        if (FileExists == Source.HadFileOnLastLoad &&
            (!FileExists || WriteTime == Source.LastWriteTime))
        {
            return false;
        }

        ApplySource(Source, FileExists, WriteTime);
        return true;
    }
}

namespace RuntimeConfigRegistry
{
    std::wstring BuildExeRelativePath(const wchar_t* FileName)
    {
        wchar_t ExePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, ExePath, MAX_PATH);

        std::wstring Path(ExePath);
        const size_t Slash = Path.rfind(L'\\');

        if (Slash != std::wstring::npos)
            Path.erase(Slash + 1);
        else
            Path.clear();

        if (FileName)
            Path += FileName;

        return Path;
    }

    bool RegisterSource(const FSourceRegistration& Registration)
    {
        if (!Registration.Id ||
            !*Registration.Id ||
            Registration.Path.empty() ||
            !Registration.ResetToDefaults)
        {
            return false;
        }

        if (FindSource(Registration.Id))
            return false;

        GSources.emplace_back();
        FSourceState& Source = GSources.back();
        Source.Id = Registration.Id;
        Source.Path = Registration.Path;
        Source.CheckIntervalSeconds =
            Registration.CheckIntervalSeconds > 0.f ?
                Registration.CheckIntervalSeconds :
                0.5f;
        Source.ResetToDefaults = Registration.ResetToDefaults;
        Source.LoadFromFile = Registration.LoadFromFile;
        Source.OnReloaded = Registration.OnReloaded;
        EnsureInitialized(Source);
        return true;
    }

    bool PollAll(float DeltaTime)
    {
        bool AnyChanged = false;

        for (size_t i = 0; i < GSources.size(); ++i)
        {
            if (PollSourceInternal(GSources[i], DeltaTime))
                AnyChanged = true;
        }

        return AnyChanged;
    }

    bool PollSource(const wchar_t* Id, float DeltaTime)
    {
        FSourceState* const Source = FindSource(Id);
        return Source ? PollSourceInternal(*Source, DeltaTime) : false;
    }

    bool ForceReload(const wchar_t* Id)
    {
        FSourceState* const Source = FindSource(Id);

        if (!Source)
            return false;

        __time64_t WriteTime = 0;
        const bool FileExists = TryGetFileWriteTime(Source->Path, WriteTime);
        ApplySource(*Source, FileExists, WriteTime);
        return true;
    }

    void Reset()
    {
        GSources.clear();
        GGlobalGeneration = 0;
    }

    unsigned long long GetGlobalGeneration()
    {
        return GGlobalGeneration;
    }

    unsigned long long GetSourceGeneration(const wchar_t* Id)
    {
        const FSourceState* const Source = FindSource(Id);
        return Source ? Source->Generation : 0;
    }
}
