#pragma once

#include "CoreMinimal.h"
#include "JsonUtils.generated.h"

/**
 * Lightweight JSON helper for files under ProjectDir/Resources/...
 * Use in editor or runtime (remember to package /Resources if reading at runtime).
 */
UCLASS()
class FLYFF_API UJsonUtils : public UObject
{
    GENERATED_BODY()

public:
    // Load a JSON object (root `{ ... }`)
    static bool LoadJsonObjectFromResources(const FString& RelativePath, TSharedPtr<FJsonObject>& OutObject);

    // Load a JSON array (root `[ ... ]`)
    static bool LoadJsonArrayFromResources(const FString& RelativePath, TArray<TSharedPtr<FJsonValue>>& OutArray);
};
