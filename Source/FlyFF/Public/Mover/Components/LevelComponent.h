#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LevelComponent.generated.h"

class UExpManager;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32, NewLevel, int32, PrevLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPointsChanged, int32, Points, int32, PointsPerLevel);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ULevelComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    ULevelComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 Level = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 Points = 0; // 0..PointsPerLevel-1

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 PointsPerLevel = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Level")
    int32 MaxLevel = 60;

    UPROPERTY(BlueprintAssignable) FOnLevelChanged OnLevelChanged;
    UPROPERTY(BlueprintAssignable) FOnPointsChanged OnPointsChanged;

    UFUNCTION(BlueprintCallable, Category="Level")
    void InitializeFromConfig(int32 InLevel, int32 InPoints, int32 InPointsPerLevel, int32 InMaxLevel);

    UFUNCTION(BlueprintCallable, Category="Level")
    void AddPoints(int32 Delta);

    UFUNCTION(BlueprintCallable, Category="Level")
    bool IsMaxLevel() const { return Level >= MaxLevel; }

private:
    void LevelUpIfNeeded();
};
