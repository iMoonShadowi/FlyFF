#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FlyffCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

UCLASS()
class FLYFF_API AFlyffCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AFlyffCharacter();
    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // Camera rig
    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* Camera;

    // Camera behavior
    UPROPERTY(EditAnywhere, Category="Camera")
    float CameraYawSpeedDegPerSec = 120.f;     // A/D rotate speed when not RMB

    UPROPERTY(EditAnywhere, Category="Camera")
    float CameraAlignSpeedDegPerSec = 360.f;   // how fast W aligns camera behind character

    UPROPERTY(EditAnywhere, Category="Camera")
    float CameraFollowTurnSpeedDegPerSec = 720.f; // camera follow speed while turning (A/D)

private:
    // Movement/camera state
    float ForwardAxis = 0.f;   // +1 fwd (W), -1 back (S)
    float TurnAxis    = 0.f;   // -1 left (A), +1 right (D) — tank turn
    float LastActorYaw = 0.f;
    
    
    bool  bAutoRun    = false; // toggled by W double-tap
    bool  bFirstTickYaw = true;
    bool  bCameraFrozen = false; // RMB held → free-look; A/D won’t rotate camera

    // W double-tap detection
    double LastWPressTime = -1.0;
    double DoubleTapSecs  = 0.30;

    // Key handlers
    void W_Pressed();  void W_Released();
    void S_Pressed();  void S_Released();
    void A_Pressed();  void A_Released();
    void D_Pressed();  void D_Released();
    void Space_Pressed(); void Space_Released();
    void RMB_Pressed();   void RMB_Released();
    void LMB_Pressed();   // click-to-move

    // Per-tick logic
    void ApplyTurn(float DeltaSeconds);
    void ApplyMovement(float DeltaSeconds);
    void ApplyFreeLook(float DeltaSeconds);
    void UpdateCameraFollow(float DeltaSeconds); // new helper (replaces your Align function)

    static float ClampAxis(float V){ return FMath::Clamp(V, -1.f, 1.f); }
};