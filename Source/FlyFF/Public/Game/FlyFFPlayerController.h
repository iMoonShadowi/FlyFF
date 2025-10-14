#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "FlyFFPlayerController.generated.h"

UCLASS()
class AFlyFFPlayerController : public APlayerController
{
  GENERATED_BODY()
public:
  AFlyFFPlayerController() { bReplicates = true; }
};
