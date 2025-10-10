#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/Data/ItemTypes.h"
#include "EquipmentComponent.generated.h"

UENUM(BlueprintType)
enum class EEquipSlot : uint8
{
    Weapon, Shield,
    Helmet, Body, Gloves, Boots,
    Cloak,  Mask,

    // Optional (future-ready)
    Necklace, RingLeft, RingRight, EarringLeft, EarringRight,
    FashionHelmet, FashionBody, FashionGloves, FashionBoots,

    MAX UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FEquippedItem
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) EEquipSlot Slot = EEquipSlot::Weapon;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FItemDef Item;
    bool IsValid() const { return Item.Id != NAME_None; }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquipped, EEquipSlot, Slot, FItemDef, Item);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUnequipped, EEquipSlot, Slot);

UCLASS(ClassGroup=(Systems), meta=(BlueprintSpawnableComponent))
class FLYFF_API UEquipmentComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable) FOnEquipped   OnEquipped;
    UPROPERTY(BlueprintAssignable) FOnUnequipped OnUnequipped;

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool CanEquip(const FItemDef& Item, EEquipSlot Slot) const;

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool Equip(const FItemDef& Item, EEquipSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool Unequip(EEquipSlot Slot);

    UFUNCTION(BlueprintCallable, Category="Equipment")
    bool IsEquipped(EEquipSlot Slot) const { return Equipped.Contains(Slot); }

    const TMap<EEquipSlot, FEquippedItem>& GetAll() const { return Equipped; }

private:
    UPROPERTY(VisibleAnywhere, Category="Equipment")
    TMap<EEquipSlot, FEquippedItem> Equipped;
};
