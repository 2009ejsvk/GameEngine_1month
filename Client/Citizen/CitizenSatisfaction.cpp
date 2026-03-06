#include "CitizenSatisfaction.h"
#include <algorithm>

namespace CitizenSatisfaction
{
    void RecalculateOverall(FNpcSatisfaction& Satisfaction)
    {
        const float WeightedAverage =
            Satisfaction.Food     * 0.18f +
            Satisfaction.Health   * 0.14f +
            Satisfaction.Fun      * 0.10f +
            Satisfaction.Faith    * 0.10f +
            Satisfaction.Housing  * 0.13f +
            Satisfaction.Job      * 0.13f +
            Satisfaction.Freedom  * 0.10f +
            Satisfaction.Security * 0.12f;

        float MinimumNeed = Satisfaction.Food;
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Health);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Fun);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Faith);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Housing);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Job);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Freedom);
        MinimumNeed = (std::min)(MinimumNeed, Satisfaction.Security);

        const float LowNeedPenalty =
            (std::max)(0.f, 30.f - MinimumNeed) * 0.8f;

        Satisfaction.Overall =
            (std::max)(0.f, (std::min)(100.f, WeightedAverage - LowNeedPenalty));
    }
}
