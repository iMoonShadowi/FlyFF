#include "Systems/Data/JsonUtils.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

static FString ResourcesPath()
{
    // Project/Resources
    return FPaths::Combine(FPaths::ProjectDir(), TEXT("Resources"));
}

bool UJsonUtils::LoadJsonObjectFromResources(const FString& RelativePath, TSharedPtr<FJsonObject>& OutObject)
{
    OutObject.Reset();

    const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(ResourcesPath(), RelativePath));
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("LoadJsonObject: couldn't read file: %s"), *FullPath);
        return false;
    }

    TSharedPtr<FJsonObject> RootObj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("LoadJsonObject: invalid JSON: %s"), *FullPath);
        return false;
    }

    OutObject = RootObj;
    return true;
}

bool UJsonUtils::LoadJsonArrayFromResources(const FString& RelativePath, TArray<TSharedPtr<FJsonValue>>& OutArray)
{
    OutArray.Reset();

    const FString FullPath = FPaths::ConvertRelativePathToFull(FPaths::Combine(ResourcesPath(), RelativePath));
    FString Json;
    if (!FFileHelper::LoadFileToString(Json, *FullPath))
    {
        UE_LOG(LogTemp, Error, TEXT("LoadJsonArray: couldn't read file: %s"), *FullPath);
        return false;
    }

    TArray<TSharedPtr<FJsonValue>> RootArray;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Json);
    if (!FJsonSerializer::Deserialize(Reader, RootArray))
    {
        UE_LOG(LogTemp, Error, TEXT("LoadJsonArray: invalid JSON: %s"), *FullPath);
        return false;
    }

    OutArray = MoveTemp(RootArray);
    return true;
}
