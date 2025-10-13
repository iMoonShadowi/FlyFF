#include "DBManager.h"
#include "Providers/PostgresProvider.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformProcess.h"
#include "Database/DatabaseProvider.h" // ensure the IDatabaseProvider is visible

void UDBManager::Initialize(FSubsystemCollectionBase& Collection)
{
    FString Err;
    Provider = MakeUnique<FPostgresProvider>();
    const FString Conn = ResolveConnString();

    bReady = Provider && Provider->Init(Conn, Err) && Provider->EnsureSchema(Err);
    if (!bReady) UE_LOG(LogTemp, Error, TEXT("Postgres init failed: %s"), *Err);

    StartWorker();
    OnDBReady.Broadcast(bReady);
}

void UDBManager::Deinitialize()
{
    StopWorker();
    if (Provider) { Provider->Shutdown(); Provider.Reset(); }
}

FString UDBManager::ResolveConnString() const
{
    // 1) Env var
    FString Env = FPlatformMisc::GetEnvironmentVariable(TEXT("FLYFF_PG"));
    if (!Env.IsEmpty()) return Env;

    // 2) DefaultGame.ini
    FString IniVal;
    if (GConfig->GetString(TEXT("/Script/Engine.GameEngine"), TEXT("FlyFFPostgres"), IniVal, GGameIni))
    {
        return IniVal;
    }

    // 3) Fallback (localhost dev)
    return TEXT("host=127.0.0.1 port=5432 dbname=flyff user=flyff password=flyff sslmode=disable");
}

void UDBManager::StartWorker()
{
    bRunWorker = true;
    WorkerThread = TUniquePtr<FRunnableThread>(FRunnableThread::Create([this]()
    {
        while (bRunWorker)
        {
            FDBJob Job;
            if (JobQueue.Dequeue(Job)) { Job.Fn(); }
            FPlatformProcess::Sleep(0.002f);
        }
    }, TEXT("FlyFF-DBWorker")));
}

void UDBManager::StopWorker()
{
    bRunWorker = false;
    if (WorkerThread) { WorkerThread->Kill(true); WorkerThread.Reset(); }
}

bool UDBManager::LoadInventory(const FGuid& CharacterId, TArray<FItemInstanceRow>& OutItems, FString& OutError)
{
    return Provider ? Provider->LoadInventory(CharacterId, OutItems, OutError) : false;
}

bool UDBManager::LoadEquipment(const FGuid& CharacterId, TMap<FName, FItemInstanceRow>& OutEquipped, FString& OutError)
{
    return Provider ? Provider->LoadEquipment(CharacterId, OutEquipped, OutError) : false;
}

void UDBManager::AsyncSaveInventory(const FGuid& CharacterId, const TArray<FItemInstanceRow>& Items)
{
    if (!Provider) return;
    TArray<FItemInstanceRow> Copy = Items;
    JobQueue.Enqueue({ [P=Provider.Get(), CharacterId, Copy]()
    {
        FString Err;
        if (!P->SaveInventory(CharacterId, Copy, Err))
            UE_LOG(LogTemp, Error, TEXT("SaveInventory failed: %s"), *Err);
    }});
}

void UDBManager::AsyncSaveEquipment(const FGuid& CharacterId, const TMap<FName, FItemInstanceRow>& Equipped)
{
    if (!Provider) return;
    TMap<FName, FItemInstanceRow> Copy = Equipped;
    JobQueue.Enqueue({ [P=Provider.Get(), CharacterId, Copy]()
    {
        FString Err;
        if (!P->SaveEquipment(CharacterId, Copy, Err))
            UE_LOG(LogTemp, Error, TEXT("SaveEquipment failed: %s"), *Err);
    }});
}
bool UDBManager::UpsertCharacterSync(const FGuid& CharacterId, const FString& Name, int32 Level, FString& OutError)
{
    if (!Provider) { OutError = TEXT("DB provider not initialized"); return false; }
    return Provider->UpsertCharacter(CharacterId, Name, Level, OutError);
}

void UDBManager::AsyncUpsertCharacter(const FGuid& CharacterId, const FString& Name, int32 Level)
{
    if (!Provider) return;
    const FString NameCopy = Name; // capture by value for the lambda
    JobQueue.Enqueue({ [P=Provider.Get(), CharacterId, NameCopy, Level]()
    {
        FString Err;
        if (!P->UpsertCharacter(CharacterId, NameCopy, Level, Err))
        {
            UE_LOG(LogTemp, Error, TEXT("UpsertCharacter failed: %s"), *Err);
        }
    }});
}

// OPTIONAL: only if you decide to store level points in a table.
// For now, you can stub it out or implement a Characters.LevelPoints column.
bool UDBManager::SaveCharacterProgressSync(const FGuid& CharacterId, int32 Level, int32 Points, FString& OutError)
{
    if (!Provider) { OutError = TEXT("DB provider not initialized"); return false; }
    // Option A: fold into UpsertCharacter by extending the provider.
    // Option B: add a new provider method, e.g., UpsertCharacterProgress(...)
    // For now, call UpsertCharacter to persist Level only:
    return Provider->UpsertCharacter(CharacterId, /*Name ignored here?*/ TEXT(""), Level, OutError);
}

void UDBManager::AsyncSaveCharacterProgress(const FGuid& CharacterId, int32 Level, int32 Points)
{
    if (!Provider) return;
    JobQueue.Enqueue({ [P=Provider.Get(), CharacterId, Level, Points]()
    {
        FString Err;
        // See note above: extend provider if you want to store Points too.
        if (!P->UpsertCharacter(CharacterId, TEXT(""), Level, Err))
        {
            UE_LOG(LogTemp, Error, TEXT("SaveCharacterProgress failed: %s"), *Err);
        }
    }});
}