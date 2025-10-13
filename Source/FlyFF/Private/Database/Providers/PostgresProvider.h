#pragma once
#include "DatabaseProvider.h"

struct pg_conn; struct pg_result;

class FPostgresProvider : public IDatabaseProvider
{
public:
    virtual ~FPostgresProvider() override;
    bool Init(const FString& ConnString, FString& OutError) override;
    void Shutdown() override;
    bool EnsureSchema(FString& OutError) override;

    bool UpsertCharacter(const FGuid& CharacterId, const FString& Name, int32 Level, FString& OutError) override;

    bool LoadInventory(const FGuid& CharacterId, TArray<FItemInstanceRow>& Out, FString& OutError) override;
    bool SaveInventory(const FGuid& CharacterId, const TArray<FItemInstanceRow>& Items, FString& OutError) override;

    bool LoadEquipment(const FGuid& CharacterId, TMap<FName, FItemInstanceRow>& Out, FString& OutError) override;
    bool SaveEquipment(const FGuid& CharacterId, const TMap<FName, FItemInstanceRow>& Eq, FString& OutError) override;

private:
    pg_conn* Conn = nullptr;

    bool Exec(const FString& Sql, FString& OutError);
    bool ExecParams(const char* Sql, int NParams, const Oid* Types, const char* const* Values,
                    const int* Lengths, const int* Formats, int ResultFormat, FString& OutError);

    static FString GuidStr(const FGuid& G);
    static FString MapIntToJson(const TMap<FName,int32>& M);
    static FString MapFloatToJson(const TMap<FName,float>& M);
};
