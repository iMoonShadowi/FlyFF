#include "Systems/Inventory/EquipmentComponent.h"

static bool IsWeaponSlot(EEquipSlot S)
{
    return S == EEquipSlot::Weapon || S == EEquipSlot::Shield;
}

bool UEquipmentComponent::CanEquip(const FItemDef& Item, EEquipSlot Slot) const
{
    switch (Item.Type)
    {
    case EItemType::Weapon:
        // 2H: only Weapon slot; 1H: Weapon or Shield
        if (Item.Handedness == EWeaponHandedness::TwoHanded)
            return Slot == EEquipSlot::Weapon;
        return IsWeaponSlot(Slot);

    case EItemType::Shield:
        return Slot == EEquipSlot::Shield;

    case EItemType::Armor:
        return (Slot == EEquipSlot::Helmet || Slot == EEquipSlot::Body ||
                Slot == EEquipSlot::Gloves || Slot == EEquipSlot::Boots ||
                Slot == EEquipSlot::Cloak  || Slot == EEquipSlot::Mask);

    case EItemType::Fashion:
        return (Slot == EEquipSlot::FashionHelmet || Slot == EEquipSlot::FashionBody ||
                Slot == EEquipSlot::FashionGloves || Slot == EEquipSlot::FashionBoots);

    case EItemType::Accessory:
        return (Slot == EEquipSlot::Necklace || Slot == EEquipSlot::RingLeft || Slot == EEquipSlot::RingRight ||
                Slot == EEquipSlot::EarringLeft || Slot == EEquipSlot::EarringRight);

    default:
        return false;
    }
}

bool UEquipmentComponent::Equip(const FItemDef& Item, EEquipSlot Slot)
{
    if (!CanEquip(Item, Slot)) return false;

    // Two-handed occupies Weapon (and clears Shield)
    if (Item.Type == EItemType::Weapon && Item.Handedness == EWeaponHandedness::TwoHanded)
    {
        // force to Weapon slot
        Slot = EEquipSlot::Weapon;

        if (Equipped.Contains(EEquipSlot::Shield))
        {
            Equipped.Remove(EEquipSlot::Shield);
            OnUnequipped.Broadcast(EEquipSlot::Shield);
        }
    }

    Equipped.Add(Slot, {Slot, Item});
    OnEquipped.Broadcast(Slot, Item);
    return true;
}

bool UEquipmentComponent::Unequip(EEquipSlot Slot)
{
    if (Equipped.Remove(Slot) > 0)
    {
        OnUnequipped.Broadcast(Slot);
        return true;
    }
    return false;
}
