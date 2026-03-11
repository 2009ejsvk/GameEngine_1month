#pragma once

#include "CitizenInfoDataProvider.h"
#include <memory>

class CWorld;

namespace CitizenInfoQueryService
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World);
}
