#include "Mover/Components/LevelComponent.h"
#include "Systems/Data/ExpManager.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

ULevelComponent::ULevelComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void ULevelComponent::InitializeFromConfig(int32 InLevel, int32 InPoints, int32 InPointsPerLevel, int32 InMaxLevel)
{
    Level = InLevel;
    Points = InPoints;
    PointsPerLevel = InPointsPerLevel;
    MaxLevel = InMaxLevel;
    OnPointsChanged.Broadcast(Points, PointsPerLevel);
}

void ULevelComponent::AddPoints(int32 Delta)
{
    if (Delta <= 0 || IsMaxLevel()) return;

    const int32 PrevLevel = Level;
    Points += Delta;

    while (Points >= PointsPerLevel && !IsMaxLevel())
    {
        Points -= PointsPerLevel;
        Level += 1;
        OnLevelChanged.Broadcast(Level, PrevLevel);
    }

    // If we hit max, cap points to last bucket
    if (IsMaxLevel())
        Points = FMath::Min(Points, PointsPerLevel - 1);

    OnPointsChanged.Broadcast(Points, PointsPerLevel);
}

void ULevelComponent::LevelUpIfNeeded() { /* handled in AddPoints loop */ }
