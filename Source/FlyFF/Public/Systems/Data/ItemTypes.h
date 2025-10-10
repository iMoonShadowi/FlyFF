#pragma once
#include "CoreMinimal.h"
#include "ItemTypes.generated.h"

UENUM(BlueprintType)
enum class EItemType : uint8
{
    Consumable, Material, Weapon, Shield, Armor, Fashion, Accessory
};

UENUM(BlueprintType)
enum class EWeaponHandedness : uint8
{
    None, OneHanded, TwoHanded
};

USTRUCT(BlueprintType)
struct FItemDef
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Id;              // e.g. "sword_bronze_01"
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText DisplayName;     // UI name
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EItemType Type = EItemType::Material;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EWeaponHandedness Handedness = EWeaponHandedness::None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxStack = 99;
};
