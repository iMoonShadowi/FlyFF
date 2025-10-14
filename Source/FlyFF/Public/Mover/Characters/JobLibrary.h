#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Job.h"
#include "JobLibrary.generated.h"

UCLASS()
class UJobLibrary : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintPure)
  static FBaseStats GetBaseStats(EJob Job) {
    FBaseStats S;
    switch (Job) {
      case EJob::SAGE:    S.INT += 5;  break;
      case EJob::WARRIOR: S.STR += 5;  break;
      case EJob::SUPPORT: S.INT += 3; S.DEX += 2; break;
      case EJob::TANK:    S.VIT += 6; S.DEX -= 1; break;
      default: break;
    }
    return S;
  }
};
