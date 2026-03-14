#include "BuildMenuQueryService.h"
#include "../World/BuildMenuWorldQuerySource.h"

namespace BuildMenuQueryService
{
    std::shared_ptr<BuildMenuDataProvider::IBuildMenuQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        return BuildMenuWorldQuerySource::CreateWorldQuerySource(World);
    }
}
