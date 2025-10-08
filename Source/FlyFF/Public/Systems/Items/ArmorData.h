#pragma once
#include "CoreMinimal.h"
#include "Systems/Data/StatTypes.h"   
#include "Systems/Items/ItemData.h"
#include "ArmorData.generated.h"

UCLASS(BlueprintType)
class FLYFF_API UArmorData : public UItemData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor")
    EArmorSlot ArmorSlot = EArmorSlot::Body;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Armor")
    FDefenseStats Defense;

    UArmorData()
    {
        Category = EItemCategory::Armor;
        // You can map ArmorSlot -> PrimarySlot at equip time, or here if you prefer.
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
