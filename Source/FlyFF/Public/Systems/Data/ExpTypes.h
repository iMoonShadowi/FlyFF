#pragma once
#include "CoreMinimal.h"
#include "ExpTypes.generated.h"

USTRUCT(BlueprintType)
struct FMonsterExpDef
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Id;        // “CrimsonWolf”
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Level = 1; // Monster template level
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 BasePoints = 1; // 1..100
};

USTRUCT(BlueprintType)
struct FExpMeta
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 MaxLevel = 60;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 PointsPerLevel = 100;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float GlobalRate = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TargetTimeToMaxHours = 6.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float AssumedKillsPerMinute = 1.5f;
};

USTRUCT(BlueprintType)
struct FExpConfig
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FExpMeta Meta;
    // key = “-2”, “+1”, “0” (delta = MonsterLevel - PlayerLevel)
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TMap<FString, float> RelativeLevelModifiers;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FMonsterExpDef> Monsters;
};
