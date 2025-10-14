#pragma once
#include "Job.generated.h"

UENUM(BlueprintType)
enum class EJob : uint8 {
  SAGE     UMETA(DisplayName="Sage"),
  WARRIOR  UMETA(DisplayName="Warrior"),
  SUPPORT  UMETA(DisplayName="Support"),
  TANK     UMETA(DisplayName="Tank")
};

USTRUCT(BlueprintType)
struct FBaseStats {
  GENERATED_BODY()
  UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 STR = 5;
  UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 INT = 5;
  UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 VIT = 5;
  UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 DEX = 5;
};
