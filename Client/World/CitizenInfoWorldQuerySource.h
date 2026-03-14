#pragma once

#include "../UI/CitizenInfoDataProvider.h"
#include <memory>

class CWorld;

namespace CitizenInfoWorldQuerySource
{
    std::shared_ptr<CitizenInfoDataProvider::ICitizenInfoQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World);
}
