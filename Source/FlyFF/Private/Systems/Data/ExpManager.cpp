#include "Systems/Data/ExpManager.h"
#include "Systems/Data/ExpConfig.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFilemanager.h"

DEFINE_LOG_CATEGORY(LogExp);

void UExpManager::Initialize(FSubsystemCollectionBase& Collection)
{
    LoadConfig();
}

FString UExpManager::ConfigPath()
{
    // Project/Content/Data/Exp/ExpConfig.json (editable)
    return FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir() / TEXT("Data/Exp/ExpConfig.json"));
}

void UExpManager::LoadConfig()
{
    FString Err;
    bLoaded = ExpConfigUtil::LoadFromJsonFile(ConfigPath(), Config, Err);
    if (!bLoaded)
    {
        UE_LOG(LogExp, Error, TEXT("EXP config failed to load: %s"), *Err);
        // Fallback defaults
        Config.Meta.MaxLevel = 60;
        Config.Meta.PointsPerLevel = 100;
        Config.RelativeLevelModifiers.Add(TEXT("0"), 1.0f);
    }
    else
    {
        UE_LOG(LogExp, Log, TEXT("EXP config loaded: MaxLvl=%d, PtsPerLvl=%d"),
            Config.Meta.MaxLevel, Config.Meta.PointsPerLevel);
    }
}

int32 UExpManager::ComputeKillPoints(int32 PlayerLevel, FName MonsterId, int32 MonsterLevelOverride) const
{
    const FMonsterExpDef* M = ExpConfigUtil::FindMonster(Config, MonsterId);
    if (!M) return 0;

    const int32 MLevel = (MonsterLevelOverride != INDEX_NONE) ? MonsterLevelOverride : M->Level;
    const int32 Delta = MLevel - PlayerLevel;

    const float Rel = ExpConfigUtil::GetRelativeModifier(Config, Delta);
    const float Raw = float(M->BasePoints) * Rel * Config.Meta.GlobalRate;

    // Clamp to [0, PointsPerLevel] to avoid silly overshoots, but allow 0 for too-low monsters
    const int32 Award = FMath::Clamp<int32>(FMath::RoundToInt(Raw), 0, Config.Meta.PointsPerLevel);
    return Award;
}
