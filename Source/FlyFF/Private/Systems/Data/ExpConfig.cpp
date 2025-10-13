#include "Systems/Data/ExpConfig.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

using namespace ExpConfigUtil;

static bool ReadFloatMap(const TSharedPtr<FJsonObject>& Obj, const FString& Field, TMap<FString,float>& Out)
{
    Out.Reset();
    const TSharedPtr<FJsonObject>* SubObj;
    if (!Obj->TryGetObjectField(Field, SubObj)) return false;
    for (const auto& Pair : (*SubObj)->Values)
    {
        double Val = 0.0;
        if (Pair.Value->TryGetNumber(Val))
            Out.Add(Pair.Key, static_cast<float>(Val));
    }
    return true;
}

bool LoadFromJsonFile(const FString& Path, FExpConfig& Out, FString& Err)
{
    FString Raw;
    if (!FFileHelper::LoadFileToString(Raw, *Path))
    {
        Err = FString::Printf(TEXT("Failed to read: %s"), *Path);
        return false;
    }

    TSharedPtr<FJsonObject> Root;
    auto Reader = TJsonReaderFactory<>::Create(Raw);
    if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
    {
        Err = TEXT("Invalid JSON");
        return false;
    }

    // Meta
    const TSharedPtr<FJsonObject>* MetaObj;
    if (Root->TryGetObjectField(TEXT("Meta"), MetaObj))
    {
        Out.Meta.MaxLevel = (*MetaObj)->GetIntegerField(TEXT("MaxLevel"));
        Out.Meta.PointsPerLevel = (*MetaObj)->GetIntegerField(TEXT("PointsPerLevel"));
        (*MetaObj)->TryGetNumberField(TEXT("GlobalRate"), Out.Meta.GlobalRate);
        (*MetaObj)->TryGetNumberField(TEXT("TargetTimeToMaxHours"), Out.Meta.TargetTimeToMaxHours);
        (*MetaObj)->TryGetNumberField(TEXT("AssumedKillsPerMinute"), Out.Meta.AssumedKillsPerMinute);
    }

    // RelativeLevelModifiers
    ReadFloatMap(Root, TEXT("RelativeLevelModifiers"), Out.RelativeLevelModifiers);

    // Monsters
    Out.Monsters.Reset();
    const TArray<TSharedPtr<FJsonValue>>* MonstersArr;
    if (Root->TryGetArrayField(TEXT("Monsters"), MonstersArr))
    {
        for (const auto& V : *MonstersArr)
        {
            const TSharedPtr<FJsonObject>* MObj;
            if (!V->TryGetObject(MObj)) continue;
            FMonsterExpDef M;
            FString IdStr;
            (*MObj)->TryGetStringField(TEXT("Id"), IdStr);
            M.Id = FName(*IdStr);
            M.Level = (*MObj)->GetIntegerField(TEXT("Level"));
            M.BasePoints = (*MObj)->GetIntegerField(TEXT("BasePoints"));
            Out.Monsters.Add(M);
        }
    }
    return true;
}

float ExpConfigUtil::GetRelativeModifier(const FExpConfig& Cfg, int32 Delta)
{
    // Look for exact key “+2” or “-3”, else clamp to nearest end
    auto GetKey = [](int32 D){ return (D>=0) ? FString::Printf(TEXT("+%d"), D) : FString::Printf(TEXT("%d"), D); };
    const FString Exact = GetKey(Delta);
    if (const float* P = Cfg.RelativeLevelModifiers.Find(Exact)) return *P;

    // find numeric key bounds
    int32 MinKey = INT32_MAX, MaxKey = INT32_MIN;
    float MinVal = 1.f, MaxVal = 1.f;
    for (const auto& KVP : Cfg.RelativeLevelModifiers)
    {
        int32 K = FCString::Atoi(*KVP.Key);
        if (K < MinKey) { MinKey = K; MinVal = KVP.Value; }
        if (K > MaxKey) { MaxKey = K; MaxVal = KVP.Value; }
    }
    if (Delta <= MinKey) return MinVal;
    if (Delta >= MaxKey) return MaxVal;

    // linear interpolate between nearest integer keys
    float AVal = 1.f; int32 AKey = 0, BKey = 0; float BVal = 1.f;
    // Find neighbors
    int32 ClosestBelow = MinKey; float ClosestBelowVal = MinVal;
    int32 ClosestAbove = MaxKey; float ClosestAboveVal = MaxVal;
    for (const auto& KVP : Cfg.RelativeLevelModifiers)
    {
        int32 K = FCString::Atoi(*KVP.Key);
        if (K <= Delta && K > ClosestBelow) { ClosestBelow = K; ClosestBelowVal = KVP.Value; }
        if (K >= Delta && K < ClosestAbove) { ClosestAbove = K; ClosestAboveVal = KVP.Value; }
    }
    if (ClosestAbove == ClosestBelow) return ClosestBelowVal;
    const float T = float(Delta - ClosestBelow) / float(ClosestAbove - ClosestBelow);
    return FMath::Lerp(ClosestBelowVal, ClosestAboveVal, T);
}

const FMonsterExpDef* ExpConfigUtil::FindMonster(const FExpConfig& Cfg, FName MonsterId)
{
    for (const auto& M : Cfg.Monsters)
        if (M.Id == MonsterId) return &M;
    return nullptr;
}
