#include "Systems/Items/WeaponData.h"

FPrimaryAssetId UWeaponData::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType Type(TEXT("Weapon"));
    return FPrimaryAssetId(Type, ItemId.IsNone() ? GetFName() : ItemId);
}
