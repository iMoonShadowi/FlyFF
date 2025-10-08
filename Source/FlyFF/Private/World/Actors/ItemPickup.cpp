#include "World/Actors/ItemPickup.h"
#include "Components/StaticMeshComponent.h"
#include "Systems/Items/ItemData.h"

AItemPickup::AItemPickup()
{
    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    SetRootComponent(Mesh);

    // Simple collision so we can overlap/click later if we want
    Mesh->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
    Mesh->SetGenerateOverlapEvents(true);
}

void AItemPickup::ApplyItemVisual()
{
    if (!Item) return;

    if (UStaticMesh* Loaded = Item->WorldMesh.LoadSynchronous())
    {
        Mesh->SetStaticMesh(Loaded);
    }
}

#if WITH_EDITOR
void AItemPickup::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyItemVisual(); // updates the mesh in-editor when you change the Item
}
#endif
