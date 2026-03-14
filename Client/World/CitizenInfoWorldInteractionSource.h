#pragma once

#include "../UI/CitizenInfoInteractionService.h"
#include <memory>

class CWorld;

namespace CitizenInfoWorldInteractionSource
{
    std::shared_ptr<CitizenInfoInteractionService::ICitizenInfoInteractionSource>
        CreateWorldInteractionSource(const std::shared_ptr<CWorld>& World);
}
