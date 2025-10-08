#include "Systems/Items/ShieldData.h"

FPrimaryAssetId UShieldData::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType Type(TEXT("Shield"));
    return FPrimaryAssetId(Type, ItemId.IsNone() ? GetFName() : ItemId);
}
