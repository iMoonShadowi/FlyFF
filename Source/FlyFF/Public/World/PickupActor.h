#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/Data/ItemTypes.h"
#include "PickupActor.generated.h"

class USphereComponent;

UCLASS()
class FLYFF_API APickupActor : public AActor
{
    GENERATED_BODY()
public:
    APickupActor();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    FItemDef ItemDef;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Pickup")
    int32 Count = 1;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    USphereComponent* Sphere;

    UFUNCTION()
    void OnOverlap(UPrimitiveComponent* Overlapped, AActor* Other,
                   UPrimitiveComponent* OtherComp, int32 BodyIndex,
                   bool bFromSweep, const FHitResult& Sweep);
};
