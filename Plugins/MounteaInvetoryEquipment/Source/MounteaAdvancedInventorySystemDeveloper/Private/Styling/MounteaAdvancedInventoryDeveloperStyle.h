// Copyright (C) 2025 Dominik (Pavlicek) Morse. All rights reserved.
//
// Developed for the Mountea Framework as a free tool. This solution is provided
// for use and sharing without charge. Redistribution is allowed under the following conditions:
//
// - You may use this solution in commercial products, provided the product is not 
//   this solution itself (or unless significant modifications have been made to the solution).
// - You may not resell or redistribute the original, unmodified solution.
//
// For more information, visit: https://mountea.tools

#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Styling/SlateStyle.h"

// Minimal editor-only style set (no AppStyle inheritance)
class FMounteaAdvancedInventoryDeveloperStyle : public FSlateStyleSet
{
public:
    FMounteaAdvancedInventoryDeveloperStyle();

    static void Initialize();
    static void Shutdown();

private:
    static TSharedPtr<FMounteaAdvancedInventoryDeveloperStyle> Instance;
};

#endif // WITH_EDITOR
