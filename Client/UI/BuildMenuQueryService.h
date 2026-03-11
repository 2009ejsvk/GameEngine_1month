#pragma once

#include "BuildMenuDataProvider.h"
#include <memory>

class CWorld;

namespace BuildMenuQueryService
{
    std::shared_ptr<BuildMenuDataProvider::IBuildMenuQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World);
}
