#include "Database/Providers/PostgresProvider.h"
#include "Database/DBTypes.h"
#include "Misc/Guid.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

extern "C" {
    #include "libpq-fe.h"
}

static void LogPQErr(PGconn* Ctx, const TCHAR* Ctxt)
{
    UE_LOG(LogTemp, Error, TEXT("%s: %s"), Ctxt, Ctx ? UTF8_TO_TCHAR(PQerrorMessage(Ctx)) : TEXT("no-conn"));
}

FPostgresProvider::~FPostgresProvider() { Shutdown(); }

bool FPostgresProvider::Init(const FString& ConnString, FString& OutError)
{
    FTCHARToUTF8 S(*ConnString);
    Conn = PQconnectdb(S.Get());
    if (PQstatus(Conn) != CONNECTION_OK)
    {
        OutError = UTF8_TO_TCHAR(PQerrorMessage(Conn));
        return false;
    }
    PGresult* R = PQexec(Conn, "SET client_min_messages TO WARNING;");
    if (R) PQclear(R);
    return true;
}

void FPostgresProvider::Shutdown()
{
    if (Conn) { PQfinish(Conn); Conn = nullptr; }
}

bool FPostgresProvider::Exec(const FString& Sql, FString& OutError)
{
    FTCHARToUTF8 U(*Sql);
    PGresult* Res = PQexec(Conn, U.Get());
    if (!Res) { LogPQErr(Conn, TEXT("Exec null")); OutError = TEXT("Null result"); return false; }
    auto S = PQresultStatus(Res);
    bool Ok = (S==PGRES_COMMAND_OK || S==PGRES_TUPLES_OK);
    if (!Ok) OutError = UTF8_TO_TCHAR(PQresultErrorMessage(Res));
    PQclear(Res);
    return Ok;
}

bool FPostgresProvider::ExecParams(const char* Sql, int NParams, const Oid* Types, const char* const* Values,
                                   const int* Lengths, const int* Formats, int ResultFormat, FString& OutError)
{
    PGresult* Res = PQexecParams(Conn, Sql, NParams, Types, Values, Lengths, Formats, ResultFormat);
    if (!Res) { LogPQErr(Conn, TEXT("ExecParams null")); OutError = TEXT("Null result"); return false; }
    auto S = PQresultStatus(Res);
    bool Ok = (S==PGRES_COMMAND_OK || S==PGRES_TUPLES_OK);
    if (!Ok) OutError = UTF8_TO_TCHAR(PQresultErrorMessage(Res));
    PQclear(Res);
    return Ok;
}

FString FPostgresProvider::GuidStr(const FGuid& G) { return G.ToString(EGuidFormats::DigitsWithHyphensLower); }

static FString MapToJsonImpl(TFunctionRef<void(TSharedRef<TJsonWriter<>>)> F)
{
    FString Out; TSharedRef<TJsonWriter<>> W = TJsonWriterFactory<>::Create(&Out);
    W->WriteObjectStart(); F(W); W->WriteObjectEnd(); W->Close(); return Out;
}
FString FPostgresProvider::MapIntToJson(const TMap<FName,int32>& M)
{
    return MapToJsonImpl([&](auto W){ for (auto& kv : M) W->WriteValue(kv.Key.ToString(), kv.Value); });
}
FString FPostgresProvider::MapFloatToJson(const TMap<FName,float>& M)
{
    return MapToJsonImpl([&](auto W){ for (auto& kv : M) W->WriteValue(kv.Key.ToString(), kv.Value); });
}

bool FPostgresProvider::EnsureSchema(FString& OutError)
{
    const TCHAR* SQL =
        TEXT("CREATE TABLE IF NOT EXISTS Items (")
        TEXT("  ItemId INT PRIMARY KEY, Name TEXT NOT NULL, Type TEXT NOT NULL, MaxStack INT NOT NULL DEFAULT 1);")

        TEXT("CREATE TABLE IF NOT EXISTS Characters (")
        TEXT("  CharacterId UUID PRIMARY KEY, Name TEXT NOT NULL, Level INT NOT NULL DEFAULT 1);")

        TEXT("CREATE TABLE IF NOT EXISTS ItemInstances (")
        TEXT("  InstanceId UUID PRIMARY KEY,")
        TEXT("  ItemId INT NOT NULL REFERENCES Items(ItemId),")
        TEXT("  Quantity INT NOT NULL DEFAULT 1,")
        TEXT("  OwnerId UUID NOT NULL REFERENCES Characters(CharacterId) ON DELETE CASCADE,")
        TEXT("  BagIndex INT NOT NULL,")
        TEXT("  IntModsJson JSONB,")
        TEXT("  FloatModsJson JSONB );")

        TEXT("CREATE TABLE IF NOT EXISTS CharacterEquipment (")
        TEXT("  OwnerId UUID NOT NULL REFERENCES Characters(CharacterId) ON DELETE CASCADE,")
        TEXT("  SlotName TEXT NOT NULL,")
        TEXT("  InstanceId UUID NOT NULL REFERENCES ItemInstances(InstanceId) ON DELETE CASCADE,")
        TEXT("  PRIMARY KEY (OwnerId, SlotName));")

        TEXT("CREATE INDEX IF NOT EXISTS idx_item_owner ON ItemInstances(OwnerId);")
        TEXT("CREATE INDEX IF NOT EXISTS idx_item_bag ON ItemInstances(OwnerId, BagIndex);");

    return Exec(SQL, OutError);
}

bool FPostgresProvider::UpsertCharacter(const FGuid& CharacterId, const FString& Name, int32 Level, FString& OutError)
{
    const FString Id = GuidStr(CharacterId);
    FTCHARToUTF8 UId(*Id), UName(*Name);
    std::string L = std::to_string(Level);
    const char* V[3] = { UId.Get(), UName.Get(), L.c_str() };
    const char* SQL =
        "INSERT INTO Characters(CharacterId, Name, Level) VALUES ($1::uuid,$2,$3::int) "
        "ON CONFLICT (CharacterId) DO UPDATE SET Name=EXCLUDED.Name, Level=EXCLUDED.Level;";
    return ExecParams(SQL, 3, nullptr, V, nullptr, nullptr, 0, OutError);
}

bool FPostgresProvider::SaveInventory(const FGuid& CharacterId, const TArray<FItemInstanceRow>& Items, FString& OutError)
{
    if (!Exec(TEXT("BEGIN;"), OutError)) return false;

    // Replace owner’s inventory wholesale (simple & robust)
    {
        const FString Id = GuidStr(CharacterId);
        FTCHARToUTF8 UId(*Id);
        const char* V[1] = { UId.Get() };
        if (!ExecParams("DELETE FROM ItemInstances WHERE OwnerId=$1::uuid;", 1, nullptr, V, nullptr, nullptr, 0, OutError))
        { Exec(TEXT("ROLLBACK;"), OutError); return false; }
    }

    for (const FItemInstanceRow& R : Items)
    {
        const FString Inst = GuidStr(R.InstanceId);
        const FString Owner = GuidStr(CharacterId);
        FTCHARToUTF8 UInst(*Inst), UOwner(*Owner);

        std::string ItemId = std::to_string(R.ItemId);
        std::string Qty    = std::to_string(FMath::Max(1, R.Quantity));
        std::string Bag    = std::to_string(FMath::Max(0, R.BagIndex));

        const FString IntJ = MapIntToJson(R.IntMods);
        const FString FltJ = MapFloatToJson(R.FloatMods);
        FTCHARToUTF8 UIntJ(*IntJ), UFltJ(*FltJ);

        const char* V[7] = { UInst.Get(), ItemId.c_str(), Qty.c_str(), UOwner.Get(), Bag.c_str(), UIntJ.Get(), UFltJ.Get() };

        const char* SQL =
            "INSERT INTO ItemInstances(InstanceId,ItemId,Quantity,OwnerId,BagIndex,IntModsJson,FloatModsJson)"
            "VALUES($1::uuid,$2::int,$3::int,$4::uuid,$5::int,$6::jsonb,$7::jsonb)"
            "ON CONFLICT(InstanceId) DO UPDATE SET "
            " ItemId=EXCLUDED.ItemId, Quantity=EXCLUDED.Quantity, OwnerId=EXCLUDED.OwnerId, "
            " BagIndex=EXCLUDED.BagIndex, IntModsJson=EXCLUDED.IntModsJson, FloatModsJson=EXCLUDED.FloatModsJson;";

        if (!ExecParams(SQL, 7, nullptr, V, nullptr, nullptr, 0, OutError))
        { Exec(TEXT("ROLLBACK;"), OutError); return false; }
    }

    return Exec(TEXT("COMMIT;"), OutError);
}

bool FPostgresProvider::LoadInventory(const FGuid& CharacterId, TArray<FItemInstanceRow>& Out, FString& OutError)
{
    Out.Reset();
    const FString Id = GuidStr(CharacterId);
    FTCHARToUTF8 UId(*Id);
    PGresult* Res = PQexecParams(Conn,
        "SELECT InstanceId::text, ItemId, Quantity, BagIndex "
        "FROM ItemInstances WHERE OwnerId=$1::uuid ORDER BY BagIndex ASC;",
        1, nullptr, (const char* const*)&UId.Get(), nullptr, nullptr, 0);

    if (!Res) { OutError = TEXT("Null result"); return false; }
    if (PQresultStatus(Res) != PGRES_TUPLES_OK) { OutError = UTF8_TO_TCHAR(PQresultErrorMessage(Res)); PQclear(Res); return false; }

    const int N = PQntuples(Res);
    for (int i=0;i<N;++i)
    {
        FItemInstanceRow Row;
        Row.InstanceId = FGuid(PQgetvalue(Res,i,0));
        Row.ItemId   = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,1)));
        Row.Quantity = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,2)));
        Row.BagIndex = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,3)));
        Out.Add(MoveTemp(Row));
    }
    PQclear(Res);
    return true;
}

bool FPostgresProvider::LoadEquipment(const FGuid& CharacterId, TMap<FName, FItemInstanceRow>& Out, FString& OutError)
{
    Out.Reset();
    const FString Id = GuidStr(CharacterId);
    FTCHARToUTF8 UId(*Id);
    PGresult* Res = PQexecParams(Conn,
        "SELECT CE.SlotName, II.InstanceId::text, II.ItemId, II.Quantity, II.BagIndex "
        "FROM CharacterEquipment CE JOIN ItemInstances II ON CE.InstanceId=II.InstanceId "
        "WHERE CE.OwnerId=$1::uuid;",
        1, nullptr, (const char* const*)&UId.Get(), nullptr, nullptr, 0);

    if (!Res) { OutError = TEXT("Null result"); return false; }
    if (PQresultStatus(Res) != PGRES_TUPLES_OK) { OutError = UTF8_TO_TCHAR(PQresultErrorMessage(Res)); PQclear(Res); return false; }

    const int N = PQntuples(Res);
    for (int i=0;i<N;++i)
    {
        const FString Slot = UTF8_TO_TCHAR(PQgetvalue(Res,i,0));
        FItemInstanceRow Row;
        Row.InstanceId = FGuid(PQgetvalue(Res,i,1));
        Row.ItemId   = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,2)));
        Row.Quantity = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,3)));
        Row.BagIndex = FCString::Atoi(UTF8_TO_TCHAR(PQgetvalue(Res,i,4)));
        Out.Add(FName(*Slot), MoveTemp(Row));
    }
    PQclear(Res);
    return true;
}

bool FPostgresProvider::SaveEquipment(const FGuid& CharacterId, const TMap<FName, FItemInstanceRow>& Eq, FString& OutError)
{
    if (!Exec(TEXT("BEGIN;"), OutError)) return false;

    // Clear
    {
        const FString Id = GuidStr(CharacterId);
        FTCHARToUTF8 UId(*Id);
        const char* V[1] = { UId.Get() };
        if (!ExecParams("DELETE FROM CharacterEquipment WHERE OwnerId=$1::uuid;", 1, nullptr, V, nullptr, nullptr, 0, OutError))
        { Exec(TEXT("ROLLBACK;"), OutError); return false; }
    }

    // Insert
    for (const auto& KVP : Eq)
    {
        const FString Slot = KVP.Key.ToString();
        const FString Owner = GuidStr(CharacterId);
        const FString Inst  = GuidStr(KVP.Value.InstanceId);

        FTCHARToUTF8 USlot(*Slot), UOwner(*Owner), UInst(*Inst);
        const char* V[3] = { UOwner.Get(), USlot.Get(), UInst.Get() };
        const char* SQL =
            "INSERT INTO CharacterEquipment(OwnerId,SlotName,InstanceId) "
            "VALUES($1::uuid,$2::text,$3::uuid) "
            "ON CONFLICT(OwnerId,SlotName) DO UPDATE SET InstanceId=EXCLUDED.InstanceId;";

        if (!ExecParams(SQL, 3, nullptr, V, nullptr, nullptr, 0, OutError))
        { Exec(TEXT("ROLLBACK;"), OutError); return false; }
    }

    return Exec(TEXT("COMMIT;"), OutError);
}
