#include "Systems/Items/ItemData.h"

FPrimaryAssetId UItemData::GetPrimaryAssetId() const
{
    static const FPrimaryAssetType DefaultType(TEXT("Item"));
    return FPrimaryAssetId(DefaultType, ItemId.IsNone() ? GetFName() : ItemId);
}
