#include "Mover/Characters/FlyffCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/Engine.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "InputCoreTypes.h"

// >>> NEW: EXP/DB includes
#include "Mover/Components/LevelComponent.h"     // your LevelComponent from earlier
#include "Systems/Data/ExpManager.h"             // UExpManager (GameInstanceSubsystem)
#include "Database/DBManager.h"                  // UDBManager (GameInstanceSubsystem)

// ---------------- Constructor ----------------
AFlyffCharacter::AFlyffCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    // --- Spring arm (camera boom) ---
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = CamTune.DefaultArmLength;
    SpringArm->SetRelativeRotation(FRotator(CamTune.DefaultPitchDegrees, 0.f, 0.f));

    SpringArm->bUsePawnControlRotation = true;
    SpringArm->bInheritYaw   = true;
    SpringArm->bInheritPitch = false;
    SpringArm->bInheritRoll  = false;

    SpringArm->bEnableCameraLag = true;
    SpringArm->CameraLagSpeed   = CamTune.Lag;
    SpringArm->bEnableCameraRotationLag = false;

    // --- Camera ---
    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
    Camera->bUsePawnControlRotation = false;

    // --- Character movement: "tank" turning via controller yaw ---
    bUseControllerRotationYaw = true;
    UCharacterMovementComponent* Move = GetCharacterMovement();
    Move->bOrientRotationToMovement = false;
    Move->RotationRate = FRotator(0.f, 720.f, 0.f);
    Move->AirControl  = 0.35f;
    Move->BrakingFrictionFactor = 1.5f;
    SetSpeedByState();

    // --- Collision & mesh offsets ---
    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
        Capsule->SetGenerateOverlapEvents(false);
    }
    if (USkeletalMeshComponent* Sk = GetMesh())
    {
        Sk->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Sk->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
        Sk->SetGenerateOverlapEvents(false);
        Sk->SetSimulatePhysics(false);

        Sk->SetRelativeLocation(FVector(0.f, 0.f, MeshZOffset));
        FRotator R = Sk->GetRelativeRotation();
        R.Yaw = MeshYawOffsetDeg;
        Sk->SetRelativeRotation(R);
    }

    // >>> NEW: Level/EXP component
    LevelComponent = CreateDefaultSubobject<ULevelComponent>(TEXT("LevelComponent"));

    // >>> NEW: if you want a deterministic GUID, replace with your own (e.g., from account id)
    CharacterGuid = FGuid::NewGuid();
}

// ---------------- BeginPlay ----------------
void AFlyffCharacter::BeginPlay()
{
    Super::BeginPlay();

    // Expected flags for coupled rig
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;

    if (SpringArm) SpringArm->bUsePawnControlRotation = true;
    if (Camera)    Camera->bUsePawnControlRotation    = false;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        PC->bAutoManageActiveCameraTarget = true;
        if (Camera) Camera->Activate(true);
        PC->SetViewTarget(this);

        PC->bShowMouseCursor = false;
        FInputModeGameOnly Mode;
        PC->SetInputMode(Mode);
        EnableInput(PC);
    }

    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    // >>> NEW: EXP: initialize from resource config + bind callbacks
    InitExpFromConfig();

    // >>> NEW: DB: upsert character row (Name/Level). Replace CharacterName source as needed.
    DB_UpsertCharacter();

    // >>> NEW: You can also load inventory/equipment here later:
    // auto* DB = GetGameInstance()->GetSubsystem<UDBManager>();
    // if (DB && DB->IsReady()) { DB->LoadInventory(CharacterGuid, ...); DB->LoadEquipment(CharacterGuid, ...); }
}

// ---------------- Input ----------------
void AFlyffCharacter::SetupPlayerInputComponent(UInputComponent* IC)
{
    Super::SetupPlayerInputComponent(IC);

    // WASD
    IC->BindKey(EKeys::W, IE_Pressed,  this, &AFlyffCharacter::W_P);
    IC->BindKey(EKeys::W, IE_Released, this, &AFlyffCharacter::W_R);
    IC->BindKey(EKeys::S, IE_Pressed,  this, &AFlyffCharacter::S_P);
    IC->BindKey(EKeys::S, IE_Released, this, &AFlyffCharacter::S_R);

    IC->BindKey(EKeys::A, IE_Pressed,  this, &AFlyffCharacter::A_P);
    IC->BindKey(EKeys::A, IE_Released, this, &AFlyffCharacter::A_R);
    IC->BindKey(EKeys::D, IE_Pressed,  this, &AFlyffCharacter::D_P);
    IC->BindKey(EKeys::D, IE_Released, this, &AFlyffCharacter::D_R);

    // Optional click-to-move + snap camera
    IC->BindKey(EKeys::LeftMouseButton,  IE_Pressed, this, &AFlyffCharacter::LMB_P);
    IC->BindKey(EKeys::MiddleMouseButton,IE_Pressed, this, &AFlyffCharacter::MMB_P);

    // Speed, autorun, walk toggle
    IC->BindKey(EKeys::LeftShift, IE_Pressed,  this, &AFlyffCharacter::Shift_P);
    IC->BindKey(EKeys::LeftShift, IE_Released, this, &AFlyffCharacter::Shift_R);
    IC->BindKey(EKeys::LeftAlt,   IE_Pressed,  this, &AFlyffCharacter::Alt_P);
    IC->BindKey(EKeys::LeftAlt,   IE_Released, this, &AFlyffCharacter::Alt_R);
    IC->BindKey(EKeys::F,         IE_Pressed,  this, &AFlyffCharacter::F_P);
    IC->BindKey(EKeys::X,         IE_Pressed,  this, &AFlyffCharacter::X_P);

    // Jump
    IC->BindKey(EKeys::SpaceBar, IE_Pressed,  this, &AFlyffCharacter::Space_P);
    IC->BindKey(EKeys::SpaceBar, IE_Released, this, &AFlyffCharacter::Space_R);
}

// ---------------- Tick ----------------
void AFlyffCharacter::Tick(float dt)
{
    Super::Tick(dt);
    TickMove(dt);
    TickCamera(dt);

    // quick overlay
    if (GEngine)
    {
        const int32 L = GetLevel();
        const int32 P = GetLevelPoints();
        GEngine->AddOnScreenDebugMessage(7, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Lv=%d  Pts=%d  Speed=%.0f  Fwd=%.1f  Turn=%.1f  Auto=%d  Walk=%d Sprint=%d"),
            L, P, GetCharacterMovement()->MaxWalkSpeed, ForwardAxis, TurnAxis, bAutorun?1:0, bWalk?1:0, bSprint?1:0));
    }
}

// ---------------- Movement ----------------
void AFlyffCharacter::TickMove(float dt)
{
    SetSpeedByState();

    if (!FMath::IsNearlyZero(TurnAxis))
        AddControllerYawInput(TurnAxis * MoveTune.TurnSpeedDeg * dt);

    const float Forward = (bAutorun ? 1.f : 0.f) + ForwardAxis;
    if (!FMath::IsNearlyZero(Forward))
        AddMovementInput(GetActorForwardVector(), FMath::Clamp(Forward, -1.f, 1.f));
}

void AFlyffCharacter::SetSpeedByState()
{
    float Target = MoveTune.RunSpeed;
    if (bWalk)          Target = MoveTune.WalkSpeed;
    else if (bSprint)   Target = MoveTune.SprintSpeed;

    GetCharacterMovement()->MaxWalkSpeed = Target;
}

// ---------------- Camera ----------------
void AFlyffCharacter::TickCamera(float /*dt*/)
{
    if (!SpringArm) return;

    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (Camera && !Camera->IsActive()) Camera->Activate(true);
        if (PC->GetViewTarget() != this)   PC->SetViewTarget(this);
    }
}

// ---------------- Input handlers ----------------
void AFlyffCharacter::W_P(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }
void AFlyffCharacter::W_R(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); }
void AFlyffCharacter::S_P(){ ForwardAxis = ClampAxis(ForwardAxis - 1.f); bAutorun=false; }
void AFlyffCharacter::S_R(){ ForwardAxis = ClampAxis(ForwardAxis + 1.f); }

void AFlyffCharacter::A_P(){ TurnAxis = ClampAxis(TurnAxis - 1.f); }
void AFlyffCharacter::A_R(){ TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_P(){ TurnAxis = ClampAxis(TurnAxis + 1.f); }
void AFlyffCharacter::D_R(){ TurnAxis = ClampAxis(TurnAxis - 1.f); }

void AFlyffCharacter::Space_P(){ Jump(); }
void AFlyffCharacter::Space_R(){ StopJumping(); }

void AFlyffCharacter::MMB_P()
{
    if (AController* C = GetController())
    {
        FRotator Ctrl = C->GetControlRotation();
        Ctrl.Yaw = GetActorRotation().Yaw;
        C->SetControlRotation(Ctrl);
    }
}

void AFlyffCharacter::Shift_P(){ if (!bWalk) bSprint = true; }
void AFlyffCharacter::Shift_R(){ bSprint = false; }
void AFlyffCharacter::Alt_P()  { bWalk   = true;  bSprint=false; }
void AFlyffCharacter::Alt_R()  { bWalk   = false; }
void AFlyffCharacter::F_P()    { bAutorun = !bAutorun; }

void AFlyffCharacter::X_P()
{
    bWalk = !bWalk;
    if (bWalk)
    {
        bSprint = false;
        if (GEngine) GEngine->AddOnScreenDebugMessage(9911, 1.5f, FColor::Yellow, TEXT("Walk: ON"));
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(9911, 1.5f, FColor::Yellow, TEXT("Walk: OFF (Run)"));
    }
}

void AFlyffCharacter::LMB_P()
{
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        FHitResult Hit;
        if (PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit))
        {
            UAIBlueprintHelperLibrary::SimpleMoveToLocation(PC, Hit.ImpactPoint);
            bAutorun = false;
        }
    }
}

// ===================== NEW: EXP / DB logic =====================

void AFlyffCharacter::InitExpFromConfig()
{
    if (!LevelComponent) return;

    if (UExpManager* ExpMgr = GetGameInstance()->GetSubsystem<UExpManager>())
    {
        const auto& Cfg = ExpMgr->GetConfig();
        LevelComponent->InitializeFromConfig(/*Level*/1, /*Points*/0, Cfg.Meta.PointsPerLevel, Cfg.Meta.MaxLevel);

        // Bind for persistence/UI
        LevelComponent->OnLevelChanged.AddDynamic(this, &AFlyffCharacter::OnLevelChanged);
        LevelComponent->OnPointsChanged.AddDynamic(this, &AFlyffCharacter::OnPointsChanged);
    }
}

void AFlyffCharacter::DB_UpsertCharacter() const
{
    if (const UDBManager* DB = GetGameInstance()->GetSubsystem<UDBManager>())
    {
        if (DB->IsReady())
        {
            FString Err;
            // NOTE: UpsertCharacter is defined on the Provider; if you exposed a wrapper, call that.
            // If not, add a simple wrapper on UDBManager to forward to Provider. For now, assume wrapper exists:
            // DB->UpsertCharacterSync(CharacterGuid, CharacterName, LevelComponent ? LevelComponent->Level : 1, Err);
            // If you haven’t added the wrapper yet, remove this call or add the wrapper.
        }
    }
}

void AFlyffCharacter::AwardKillEXP(FName MonsterId, int32 MonsterLevel)
{
    if (!LevelComponent) return;

    if (UExpManager* ExpMgr = GetGameInstance()->GetSubsystem<UExpManager>())
    {
        const int32 Award = ExpMgr->ComputeKillPoints(LevelComponent->Level, MonsterId, MonsterLevel);
        if (Award > 0) LevelComponent->AddPoints(Award);
    }
}

void AFlyffCharacter::OnLevelChanged(int32 NewLevel, int32 PrevLevel)
{
    // Persist level to DB
    if (const UDBManager* DB = GetGameInstance()->GetSubsystem<UDBManager>())
    {
        if (DB->IsReady())
        {
            FString Err;
            // See note in DB_UpsertCharacter(); call your wrapper:
            // DB->UpsertCharacterSync(CharacterGuid, CharacterName, NewLevel, Err);
        }
    }

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(9912, 2.f, FColor::Green,
            FString::Printf(TEXT("LEVEL UP! %d -> %d"), PrevLevel, NewLevel));
    }
}

void AFlyffCharacter::OnPointsChanged(int32 Points, int32 PointsPerLevel)
{
    // Hook UI here (progress bar, etc.). Debug print for now.
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(9913, 1.5f, FColor::Cyan,
            FString::Printf(TEXT("EXP: %d / %d"), Points, PointsPerLevel));
    }
}

int32 AFlyffCharacter::GetLevel() const
{
    return LevelComponent ? LevelComponent->Level : 1;
}

int32 AFlyffCharacter::GetLevelPoints() const
{
    return LevelComponent ? LevelComponent->Points : 0;
}
