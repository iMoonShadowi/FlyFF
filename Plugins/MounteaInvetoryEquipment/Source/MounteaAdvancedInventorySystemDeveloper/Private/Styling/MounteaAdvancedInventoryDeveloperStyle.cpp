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

#if WITH_EDITOR

#include "MounteaAdvancedInventoryDeveloperStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/AppStyle.h"   // FAppStyle::GetAppStyleSetName()

TSharedPtr<FMounteaAdvancedInventoryDeveloperStyle> FMounteaAdvancedInventoryDeveloperStyle::Instance;

FMounteaAdvancedInventoryDeveloperStyle::FMounteaAdvancedInventoryDeveloperStyle()
    : FSlateStyleSet(TEXT("MounteaAdvancedInventoryDeveloperStyle"))
{
    // Optional: set content root or register icons using the app style if desired, e.g.:
    // const FName AppStyle = FAppStyle::GetAppStyleSetName();
    // SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
    // NOTE: do NOT call SetAppStyleSet(...); not needed and not in 5.5 API.
}

void FMounteaAdvancedInventoryDeveloperStyle::Initialize()
{
    if (!Instance.IsValid())
    {
        Instance = MakeShareable(new FMounteaAdvancedInventoryDeveloperStyle());
        FSlateStyleRegistry::RegisterSlateStyle(*Instance.Get());
    }
}

void FMounteaAdvancedInventoryDeveloperStyle::Shutdown()
{
    if (Instance.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*Instance.Get());
        ensure(Instance.IsUnique());
        Instance.Reset();
    }
}

#endif // WITH_EDITOR
