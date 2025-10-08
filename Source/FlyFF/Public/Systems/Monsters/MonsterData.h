// Source/FlyFF/Public/Systems/Monsters/MonsterData.h
#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Systems/Data/StatTypes.h"
#include "MonsterData.generated.h"

UCLASS(BlueprintType)
class FLYFF_API UMonsterData : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FName MonsterId;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText DisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FStatBlock BaseStats;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Stats")
    TArray<FStatModifier> Modifiers;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<USkeletalMesh> SkeletalMesh;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TSoftObjectPtr<UAnimBlueprint> AnimBlueprint;

    // Defense/Resists
    UPROPERTY(EditAnywhere, BlueprintReadOnly) int32 Defense = 0;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float PhysResist = 0.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) float MagicResist = 0.f;

    // Loot/progression later...
};
