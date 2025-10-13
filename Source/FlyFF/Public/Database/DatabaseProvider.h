#pragma once
#include "CoreMinimal.h"

struct FItemInstanceRow;

class IDatabaseProvider
{
public:
    virtual ~IDatabaseProvider() = default;

    virtual bool Init(const FString& ConnString, FString& OutError) = 0;
    virtual void Shutdown() = 0;
    virtual bool EnsureSchema(FString& OutError) = 0;

    virtual bool UpsertCharacter(const FGuid& CharacterId, const FString& Name, int32 Level, FString& OutError) = 0;

    virtual bool LoadInventory(const FGuid& CharacterId, TArray<FItemInstanceRow>& Out, FString& OutError) = 0;
    virtual bool SaveInventory(const FGuid& CharacterId, const TArray<FItemInstanceRow>& Items, FString& OutError) = 0;

    virtual bool LoadEquipment(const FGuid& CharacterId, TMap<FName, FItemInstanceRow>& Out, FString& OutError) = 0;
    virtual bool SaveEquipment(const FGuid& CharacterId, const TMap<FName, FItemInstanceRow>& Eq, FString& OutError) = 0;
};
