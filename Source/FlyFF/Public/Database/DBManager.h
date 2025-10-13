#pragma once
#include "Subsystems/GameInstanceSubsystem.h"
#include "DBTypes.h"
#include "DBManager.generated.h"

class IDatabaseProvider;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDBReady, bool, bSuccess);

UCLASS()
class UDBManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable) FOnDBReady OnDBReady;

    bool IsReady() const { return bReady; }

    // Sync loads (call on server thread during spawn/login)
    bool LoadInventory(const FGuid& CharacterId, TArray<FItemInstanceRow>& OutItems, FString& OutError);
    bool LoadEquipment(const FGuid& CharacterId, TMap<FName, FItemInstanceRow>& OutEquipped, FString& OutError);

    // Async saves (enqueue to the DB worker)
    void AsyncSaveInventory(const FGuid& CharacterId, const TArray<FItemInstanceRow>& Items);
    void AsyncSaveEquipment(const FGuid& CharacterId, const TMap<FName, FItemInstanceRow>& Equipped);

private:
    FString ResolveConnString() const; // Env -> Ini
    TUniquePtr<IDatabaseProvider> Provider;
    bool bReady = false;

    struct FDBJob { TFunction<void()> Fn; };
    TQueue<FDBJob, EQueueMode::Mpsc> JobQueue;
    FThreadSafeBool bRunWorker = false;
    TUniquePtr<FRunnableThread> WorkerThread;

    void StartWorker();
    void StopWorker();
};
