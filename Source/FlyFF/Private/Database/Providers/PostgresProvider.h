#pragma once
#include "Database/DatabaseProvider.h"

// Make libpq types visible in the header (Oid, PGconn, etc.)
extern "C" {
    #include "libpq-fe.h"
}

class FPostgresProvider : public IDatabaseProvider
{
public:
    virtual ~FPostgresProvider() override;

    virtual bool Init(const FString& ConnString, FString& OutError) override;
    virtual void Shutdown() override;
    virtual bool EnsureSchema(FString& OutError) override;

    virtual bool UpsertCharacter(const FGuid& CharacterId, const FString& Name, int32 Level, FString& OutError) override;

    virtual bool LoadInventory(const FGuid& CharacterId, TArray<struct FItemInstanceRow>& Out, FString& OutError) override;
    virtual bool SaveInventory(const FGuid& CharacterId, const TArray<struct FItemInstanceRow>& Items, FString& OutError) override;

    virtual bool LoadEquipment(const FGuid& CharacterId, TMap<FName, struct FItemInstanceRow>& Out, FString& OutError) override;
    virtual bool SaveEquipment(const FGuid& CharacterId, const TMap<FName, struct FItemInstanceRow>& Eq, FString& OutError) override;

private:
    PGconn* Conn = nullptr; // <-- use PGconn

    bool Exec(const FString& Sql, FString& OutError);
    bool ExecParams(const char* Sql, int NParams,
                    const Oid* ParamTypes, const char* const* ParamValues,
                    const int* ParamLengths, const int* ParamFormats,
                    int ResultFormat, FString& OutError);

    static FString GuidStr(const FGuid& G);
    static FString MapIntToJson(const TMap<FName,int32>& M);
    static FString MapFloatToJson(const TMap<FName,float>& M);
};
