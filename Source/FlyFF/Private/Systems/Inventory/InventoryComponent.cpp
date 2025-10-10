#include "Systems/Inventory/InventoryComponent.h"

bool UInventoryComponent::AddItem(const FItemDef& Def, int32 Count)
{
    if (Count <= 0) return false;

    // Try stacking first
    for (FItemStack& S : Items)
    {
        if (S.Item.Id == Def.Id && S.Count < S.Item.MaxStack)
        {
            const int32 Space = S.Item.MaxStack - S.Count;
            const int32 ToAdd = FMath::Min(Space, Count);
            S.Count += ToAdd;
            Count   -= ToAdd;
            if (Count <= 0) return true;
        }
    }

    // Add new stacks while there is room
    while (Count > 0 && Items.Num() < MaxSlots)
    {
        const int32 ToAdd = FMath::Min(Def.MaxStack, Count);
        Items.Add({Def, ToAdd});
        Count -= ToAdd;
    }

    return Count == 0;
}

bool UInventoryComponent::RemoveItemById(FName Id, int32 Count)
{
    if (Count <= 0) return false;

    for (int32 i = Items.Num() - 1; i >= 0 && Count > 0; --i)
    {
        if (Items[i].Item.Id == Id)
        {
            const int32 ToRemove = FMath::Min(Items[i].Count, Count);
            Items[i].Count -= ToRemove;
            Count -= ToRemove;
            if (Items[i].Count <= 0)
            {
                Items.RemoveAt(i);
            }
        }
    }
    return Count == 0;
}

int32 UInventoryComponent::GetCountById(FName Id) const
{
    int32 Total = 0;
    for (const FItemStack& S : Items)
    {
        if (S.Item.Id == Id) Total += S.Count;
    }
    return Total;
}
