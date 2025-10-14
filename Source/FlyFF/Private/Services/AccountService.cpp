#include "Services/AccountService.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

void UAccountService::SendJson(const FString& Route, const FString& Method, const FString& Json,
                               TFunction<void(const FString&)> OnOk, TFunction<void(const FString&)> OnErr) {
  auto& Http = FHttpModule::Get();
  TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = Http.CreateRequest();
  Req->SetURL(BaseUrl + Route);
  Req->SetVerb(Method);
  Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
  if (!AuthToken.IsEmpty()) Req->SetHeader(TEXT("Authorization"), TEXT("Bearer ")+AuthToken);
  Req->SetContentAsString(Json);

  Req->OnProcessRequestComplete().BindLambda([OnOk, OnErr](FHttpRequestPtr, FHttpResponsePtr Res, bool bOk){
    if (bOk && Res.IsValid() && Res->GetResponseCode() >= 200 && Res->GetResponseCode() < 300) {
      OnOk(Res->GetContentAsString());
    } else {
      OnErr(Res.IsValid() ? FString::FromInt(Res->GetResponseCode()) + TEXT(" ") + Res->GetContentAsString()
                          : TEXT("no_response"));
    }
  });
  Req->ProcessRequest();
}

void UAccountService::Login(const FString& UsernameOrEmail, const FString& Password) {
  const FString Body = FString::Printf(TEXT("{\"usernameOrEmail\":\"%s\",\"password\":\"%s\"}"),
    *UsernameOrEmail, *Password);
  SendJson(TEXT("/api/login"), TEXT("POST"), Body,
    [this](const FString& JsonStr){
      TSharedPtr<FJsonObject> Obj; auto Reader = TJsonReaderFactory<>::Create(JsonStr);
      if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid()) {
        AuthToken = Obj->GetStringField(TEXT("token"));
        OnLogin.Broadcast(true, TEXT(""));
      } else OnLogin.Broadcast(false, TEXT("bad_json"));
    },
    [this](const FString& Err){ OnLogin.Broadcast(false, Err); });
}

void UAccountService::FetchCharacters() {
  SendJson(TEXT("/api/characters"), TEXT("GET"), TEXT(""),
    [this](const FString& JsonStr){
      TSharedPtr<FJsonObject> Obj; auto Reader = TJsonReaderFactory<>::Create(JsonStr);
      TArray<FCharacterInfo> Out; int32 Max = 3;
      if (FJsonSerializer::Deserialize(Reader, Obj) && Obj.IsValid()) {
        Max = Obj->GetIntegerField(TEXT("max"));
        const TArray<TSharedPtr<FJsonValue>>* Arr;
        if (Obj->TryGetArrayField(TEXT("characters"), Arr)) {
          for (auto& V : *Arr) {
            const auto O = V->AsObject();
            FCharacterInfo C;
            C.Id    = O->GetStringField(TEXT("id"));
            C.Name  = O->GetStringField(TEXT("name"));
            C.Job   = O->GetStringField(TEXT("job"));
            C.Level = (int32)O->GetNumberField(TEXT("level"));
            Out.Add(C);
          }
        }
      }
      OnCharacters.Broadcast(Out, Max);
    },
    [](const FString&){});
}

void UAccountService::CreateCharacter(const FString& Name, const FString& Job) {
  const FString Body = FString::Printf(TEXT("{\"name\":\"%s\",\"job\":\"%s\"}"), *Name, *Job);
  SendJson(TEXT("/api/characters"), TEXT("POST"), Body,
    [this](const FString&){ FetchCharacters(); },
    [](const FString&){});
}

void UAccountService::StartCharacterSession(const FString& CharacterId) {
  const FString Body = FString::Printf(TEXT("{\"characterId\":\"%s\"}"), *CharacterId);
  SendJson(TEXT("/api/session"), TEXT("POST"), Body,
    [](const FString&){ /* TODO: pass token to your game server handshake */ },
    [](const FString&){});
}
