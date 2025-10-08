#include "Systems/Items/ArmorData.h"

FPrimaryAssetId UArmorData::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType Type(TEXT("Armor"));
    return FPrimaryAssetId(Type, ItemId.IsNone() ? GetFName() : ItemId);
}
