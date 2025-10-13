#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "ExpTypes.h"
#include "ExpManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogExp, Log, All);

UCLASS()
class UExpManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Returns absolute file path (you can change this if you prefer inside Saved/)
    static FString ConfigPath();

    const FExpConfig& GetConfig() const { return Config; }
    bool IsLoaded() const { return bLoaded; }

    // Award points for a kill. Simple call from your death handler.
    // PlayerLevel: current player level
    // MonsterId:   id key from JSON (e.g., "CrimsonWolf")
    // MonsterLevel: if you prefer to pass the live monster level instead of template level,
    //               pass it; otherwise set to INDEX_NONE to use template level from config.
    int32 ComputeKillPoints(int32 PlayerLevel, FName MonsterId, int32 MonsterLevel = INDEX_NONE) const;

private:
    FExpConfig Config;
    bool bLoaded = false;
    void LoadConfig();
};
