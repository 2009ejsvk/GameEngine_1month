#include "CitizenInfoInteractionService.h"
#include "../World/CitizenInfoWorldInteractionSource.h"

namespace CitizenInfoInteractionService
{
    std::shared_ptr<ICitizenInfoInteractionSource>
        CreateWorldInteractionSource(const std::shared_ptr<CWorld>& World)
    {
        return CitizenInfoWorldInteractionSource::CreateWorldInteractionSource(
            World);
    }
}
