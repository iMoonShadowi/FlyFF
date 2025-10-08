#include "Systems/Items/FashionData.h"

FPrimaryAssetId UFashionData::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType Type(TEXT("Fashion"));
    return FPrimaryAssetId(Type, ItemId.IsNone() ? GetFName() : ItemId);
}
