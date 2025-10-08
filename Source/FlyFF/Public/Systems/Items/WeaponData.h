#pragma once
#include "CoreMinimal.h"
#include "Systems/Items/ItemData.h"
#include "Systems/Data/StatTypes.h"
#include "WeaponData.generated.h"

UCLASS(BlueprintType)
class FLYFF_API UWeaponData : public UItemData
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
    EWeaponType WeaponType = EWeaponType::Sword1H;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
    FAttackStats Attack;

    UWeaponData()
    {
        Category    = EItemCategory::Weapon;
        PrimarySlot = EEquipmentSlot::WeaponMainHand;
    }

    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
