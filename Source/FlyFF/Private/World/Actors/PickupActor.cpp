#include "World/PickupActor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Systems/Inventory/InventoryComponent.h"

APickupActor::APickupActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->InitSphereRadius(80.f);
    Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    Sphere->SetCollisionResponseToAllChannels(ECR_Overlap);
    RootComponent = Sphere;

    Sphere->OnComponentBeginOverlap.AddDynamic(this, &APickupActor::OnOverlap);
}

void APickupActor::OnOverlap(UPrimitiveComponent*, AActor* Other, UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    if (ACharacter* Ch = Cast<ACharacter>(Other))
    {
        if (UInventoryComponent* Inv = Ch->FindComponentByClass<UInventoryComponent>())
        {
            if (Inv->AddItem(ItemDef, Count))
            {
                Destroy();
            }
        }
    }
}
