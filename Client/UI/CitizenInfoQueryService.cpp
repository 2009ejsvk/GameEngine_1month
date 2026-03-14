#include "CitizenInfoQueryService.h"
#include "../World/CitizenInfoWorldQuerySource.h"

namespace CitizenInfoQueryService
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World)
    {
        return CitizenInfoWorldQuerySource::CreateWorldQuerySource(World);
    }
}
