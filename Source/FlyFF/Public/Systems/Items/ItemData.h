// Source/FlyFF/Public/Systems/Items/ItemData.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Systems/Data/StatTypes.h"
#include "ItemData.generated.h"

UCLASS(BlueprintType, Abstract)
class FLYFF_API UItemData : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    EItemCategory Category = EItemCategory::Material;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    FName ItemId; // stable ID (used in JSON/shops)

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Equip")
    EEquipmentSlot PrimarySlot = EEquipmentSlot::ArmorBody; // will be overridden in subclasses

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    FStatBlock Stats;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    TArray<FStatModifier> Modifiers;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
    TSoftObjectPtr<UStaticMesh> WorldMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Visual")
    TSoftObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Economy")
    int32 BasePrice = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tags")
    FGameplayTagContainer Tags;

    // Base type name ("Item") if a subclass doesn't override.
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};
