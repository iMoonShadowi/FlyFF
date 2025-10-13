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
#include "Interfaces/IPluginManager.h"
#include "UObject/Interface.h"
#include "UObject/UObjectIterator.h"          // TObjectIterator
#include "Kismet/BlueprintFunctionLibrary.h"  // UBlueprintFunctionLibrary
#include "UObject/Interface.h"                // UInterface
#include "Misc/PackageName.h"                 // FPackageName
#include "UObject/Package.h"                  // UPackage (GetPackage)

enum class EFunctionCallType
{
	Function,
	Message,
	Delegate,
	Unknown
};

enum class EFunctionRole
{
	Set,
	Get,
	Validate,
	Unknown
};

namespace MounteaAdvancedInventoryHelpers
{
	TSharedPtr<IPlugin> GetThisPlugin();
	const TSet<UClass*> GetRelevantClasses();

	inline TSharedPtr<IPlugin> GetThisPlugin()
	{
		return IPluginManager::Get().FindPlugin(TEXT("MounteaAdvancedInventorySystem"));
	}

	inline const TSet<UClass*> GetRelevantClasses()
	{
		static TSet<UClass*> relevantClasses;
		
		if (!relevantClasses.IsEmpty())
		{
			return relevantClasses;
		}

		const TSharedPtr<IPlugin> thisPlugin = GetThisPlugin();
		if (!thisPlugin.IsValid())
		{
			return relevantClasses;
		}

		TSet<FString> thisPluginModulesNames;
		for (const FModuleDescriptor& module : thisPlugin->GetDescriptor().Modules)
		{
			thisPluginModulesNames.Add(module.Name.ToString());
		}
		
const UClass* const BlueprintFunctionLibraryClass = UBlueprintFunctionLibrary::StaticClass();
const UClass* const InterfaceClass                = UInterface::StaticClass();

TArray<UClass*> RelevantClasses;
for (TObjectIterator<UClass> It; It; ++It)
{
    UClass* ClassType = *It;
    if (!ClassType) continue;

    UPackage* Pkg = ClassType->GetPackage();
    if (!Pkg) continue;

    const FString PackageName = FPackageName::GetLongPackageAssetName(Pkg->GetName());
    if (!thisPluginModulesNames.Contains(PackageName)) continue;

    // Include function libraries
    if (ClassType->IsChildOf(BlueprintFunctionLibraryClass))
    {
        RelevantClasses.Add(ClassType);
    }
    // Include interfaces
    else if (ClassType->IsChildOf(InterfaceClass))
    {
        RelevantClasses.Add(ClassType);
    }
    // Include regular classes
    else if (!ClassType->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
    {
        RelevantClasses.Add(ClassType);
    }
}

		return relevantClasses;
	}
}