#pragma once

#include "../UI/BuildMenuDataProvider.h"
#include <memory>

class CWorld;

namespace BuildMenuWorldQuerySource
{
    std::shared_ptr<BuildMenuDataProvider::IBuildMenuQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World);
}
