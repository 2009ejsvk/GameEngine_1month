#pragma once

#include <memory>

class CWorldUIManager;

namespace UIConfig
{
    void ApplyWidgetOverrides(const std::shared_ptr<CWorldUIManager>& UIManager);
}

namespace UILayoutApplier
{
    inline void ApplyWidgetOverrides(
        const std::shared_ptr<CWorldUIManager>& UIManager)
    {
        UIConfig::ApplyWidgetOverrides(UIManager);
    }
}
