#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "FlyFFPlayerState.generated.h"

UCLASS()
class AFlyFFPlayerState : public APlayerState
{
  GENERATED_BODY()
public:
  AFlyFFPlayerState() { bReplicates = true; }
};
