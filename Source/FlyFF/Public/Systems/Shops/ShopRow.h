// Source/FlyFF/Public/Systems/Shops/ShopRow.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ShopRow.generated.h"

USTRUCT(BlueprintType)
struct FShopRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ShopId;     // e.g., "CentralBlacksmith"
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName ItemId;     // must match UItemData::ItemId
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Price = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Quantity = -1; // -1 = infinite
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 LevelMin = 1;
};
