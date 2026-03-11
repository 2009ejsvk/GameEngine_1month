#pragma once

#include "AlmanacDataProvider.h"
#include <memory>

class CWorld;

namespace AlmanacQueryService
{
    std::shared_ptr<AlmanacDataProvider::IAlmanacQuerySource>
        CreateWorldQuerySource(const std::shared_ptr<CWorld>& World);
}
