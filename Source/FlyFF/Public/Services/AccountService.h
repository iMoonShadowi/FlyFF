#pragma once
#include "CoreMinimal.h"
#include "AccountService.generated.h"

USTRUCT(BlueprintType)
struct FCharacterInfo {
  GENERATED_BODY()
  UPROPERTY(BlueprintReadOnly) FString Id;
  UPROPERTY(BlueprintReadOnly) FString Name;
  UPROPERTY(BlueprintReadOnly) FString Job;
  UPROPERTY(BlueprintReadOnly) int32 Level = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLogin, bool, bSuccess, const FString&, Error);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacters, const TArray<FCharacterInfo>&, Characters, int32, Max);

UCLASS(BlueprintType)
class UAccountService : public UObject {
  GENERATED_BODY()
public:
  UPROPERTY(BlueprintReadWrite, EditAnywhere) FString BaseUrl = TEXT("http://127.0.0.1:4000");
  UPROPERTY(BlueprintReadOnly) FString AuthToken;

  UPROPERTY(BlueprintAssignable) FOnLogin OnLogin;
  UPROPERTY(BlueprintAssignable) FOnCharacters OnCharacters;

  UFUNCTION(BlueprintCallable) void Login(const FString& UsernameOrEmail, const FString& Password);
  UFUNCTION(BlueprintCallable) void FetchCharacters();
  UFUNCTION(BlueprintCallable) void CreateCharacter(const FString& Name, const FString& Job);
  UFUNCTION(BlueprintCallable) void StartCharacterSession(const FString& CharacterId);

private:
  void SendJson(const FString& Route, const FString& Method, const FString& Json,
                TFunction<void(const FString&)> OnOk, TFunction<void(const FString&)> OnErr);
};
