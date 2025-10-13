#pragma once
#include "CoreMinimal.h"
#include "DBTypes.generated.h"

USTRUCT(BlueprintType)
struct FItemInstanceRow
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FGuid InstanceId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ItemId = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Quantity = 1;

    // Optional: use if you track a bag/slot index in your Mountea adapter
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BagIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, int32> IntMods;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FName, float> FloatMods;
};
