#pragma once
#include "CoreMinimal.h"
#include "ExpTypes.h"

namespace ExpConfigUtil
{
    // Loads JSON file into FExpConfig. Returns false on error, leaves Out empty.
    bool LoadFromJsonFile(const FString& AbsolutePath, FExpConfig& Out, FString& OutError);

    // Gets the closest modifier by clamping to min/max keys if exact delta not found.
    float GetRelativeModifier(const FExpConfig& Cfg, int32 Delta);

    const FMonsterExpDef* FindMonster(const FExpConfig& Cfg, FName MonsterId);
}
