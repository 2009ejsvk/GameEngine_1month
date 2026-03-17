#pragma once

namespace UIConfig
{
    void RegisterRuntimeConfig();
    bool ReloadIfChanged(float DeltaTime);
}

namespace UILayoutLoader
{
    inline void RegisterRuntimeConfig()
    {
        UIConfig::RegisterRuntimeConfig();
    }

    inline bool ReloadIfChanged(float DeltaTime)
    {
        return UIConfig::ReloadIfChanged(DeltaTime);
    }
}
