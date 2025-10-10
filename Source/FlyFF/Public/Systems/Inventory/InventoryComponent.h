#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/Data/ItemTypes.h"
#include "InventoryComponent.generated.h"

USTRUCT(BlueprintType)
struct FItemStack
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FItemDef Item;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Count = 1;
};

UCLASS(ClassGroup=(Systems), meta=(BlueprintSpawnableComponent))
class FLYFF_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inventory")
    int32 MaxSlots = 30;

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool AddItem(const FItemDef& Def, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    bool RemoveItemById(FName Id, int32 Count = 1);

    UFUNCTION(BlueprintCallable, Category="Inventory")
    int32 GetCountById(FName Id) const;

    const TArray<FItemStack>& GetItems() const { return Items; }

private:
    UPROPERTY(VisibleAnywhere, Category="Inventory")
    TArray<FItemStack> Items;
};
