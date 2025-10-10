#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FlyffCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

USTRUCT(BlueprintType)
struct FMoveTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float WalkSpeed    = 250.f; // requested 250
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RunSpeed     = 600.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float SprintSpeed  = 900.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float TurnSpeedDeg = 140.f; // A/D turn speed
};

USTRUCT(BlueprintType)
struct FCameraTuning
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) float FollowYawSpeed      = 360.f; // reserved
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float Lag                 = 12.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float RotLag              = 15.f;  // keep off for tight follow
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DefaultArmLength    = 450.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) float DefaultPitchDegrees = -15.f;
};

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

    // Components
    UPROPERTY(VisibleAnywhere, Category="Camera")
    USpringArmComponent* SpringArm = nullptr;

    UPROPERTY(VisibleAnywhere, Category="Camera")
    UCameraComponent* Camera = nullptr;

    // Tuning
    UPROPERTY(EditAnywhere, Category="Tuning")
    FMoveTuning MoveTune;

    UPROPERTY(EditAnywhere, Category="Tuning")
    FCameraTuning CamTune;

    // Mesh offsets (so visual forward = +X; helpful for Paragon meshes)
    UPROPERTY(EditAnywhere, Category="Mesh")
    float MeshYawOffsetDeg = -90.f;

    UPROPERTY(EditAnywhere, Category="Mesh")
    float MeshZOffset = -90.f;

private:
    // ---- State ----
    float ForwardAxis = 0.f;   // W/S
    float TurnAxis    = 0.f;   // A/D
    bool  bWalk   = false;     // toggled with X or held with Alt
    bool  bSprint = false;     // Shift
    bool  bAutorun = false;    // F

    // Input handlers
    void W_P(); void W_R(); void S_P(); void S_R();
    void A_P(); void A_R(); void D_P(); void D_R();
    void Space_P(); void Space_R();
    void MMB_P();              // snap camera behind
    void Shift_P(); void Shift_R();
    void Alt_P();   void Alt_R();
    void F_P();                // autorun toggle
    void LMB_P();              // optional click-to-move
    void X_P();                // Walk toggle

    // Per-tick
    void TickMove(float DeltaSeconds);
    void TickCamera(float DeltaSeconds);
    void SetSpeedByState();

    static float ClampAxis(float V){ return FMath::Clamp(V, -1.f, 1.f); }
};
