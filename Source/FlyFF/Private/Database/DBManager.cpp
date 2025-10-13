#include "DBManager.h"
#include "Providers/PostgresProvider.h"
#include "Misc/ConfigCacheIni.h"
#include "HAL/PlatformProcess.h"

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
