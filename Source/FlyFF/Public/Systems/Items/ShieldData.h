#pragma once
#include "CoreMinimal.h"
#include "Systems/Items/ItemData.h"
#include "Systems/Data/StatTypes.h"
#include "ShieldData.generated.h"

UCLASS(BlueprintType)
class FLYFF_API UShieldData : public UItemData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Shield")
    FDefenseStats Defense;

    UShieldData()
    {
        Category    = EItemCategory::Shield;
        PrimarySlot = EEquipmentSlot::OffHand;
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
