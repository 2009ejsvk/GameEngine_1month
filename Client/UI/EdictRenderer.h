#pragma once

#include "EdictDataProvider.h"

struct FVector2;
class CEdictWidget;

class FEdictRenderer final
{
public:
    static void CreateWidgets(CEdictWidget& Widget);
    static void ApplySnapshot(
        CEdictWidget& Widget,
        const EdictDataProvider::FEdictSnapshot& Snapshot);
    static void RefreshLayout(CEdictWidget& Widget);
    static void ApplyOpenState(CEdictWidget& Widget);
    static bool IsMouseOverPanel(
        const CEdictWidget& Widget,
        const FVector2& MousePos);
};
