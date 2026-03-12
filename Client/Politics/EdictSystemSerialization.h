#pragma once

#include "EdictSystem.h"
#include <string>
#include <vector>

namespace EdictSystemSerialization
{
    bool LoadEdictDefinitionsFromFile(
        const std::wstring& Path,
        std::vector<FGovernmentEdictDefinition>& InOutDefinitions);

    void WriteEdictDefinitionsToFile(
        const std::wstring& Path,
        const std::vector<FGovernmentEdictDefinition>& Definitions);
}
