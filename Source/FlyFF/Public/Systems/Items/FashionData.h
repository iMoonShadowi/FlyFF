#pragma once
#include "CoreMinimal.h"
#include "Systems/Data/StatTypes.h"   
#include "Systems/Items/ItemData.h"
#include "FashionData.generated.h"

UCLASS(BlueprintType)
class FLYFF_API UFashionData : public UItemData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Fashion")
    EFashionSlot FashionSlot = EFashionSlot::Body;

    UFashionData()
    {
        Category = EItemCategory::Fashion;
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};