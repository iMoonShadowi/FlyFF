#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemPickup.generated.h"

class UStaticMeshComponent;
class UItemData;

UCLASS()
class FLYFF_API AItemPickup : public AActor
{
    GENERATED_BODY()

public:
    AItemPickup();

#if WITH_EDITOR
    virtual void OnConstruction(const FTransform& Transform) override;
#endif

protected:
    UPROPERTY(VisibleAnywhere, Category="Item")
    UStaticMeshComponent* Mesh;

    UPROPERTY(EditAnywhere, Category="Item")
    UItemData* Item;

    UFUNCTION(BlueprintCallable, Category="Item")
    void ApplyItemVisual();
};
