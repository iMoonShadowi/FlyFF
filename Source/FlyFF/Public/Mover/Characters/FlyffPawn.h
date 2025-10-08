// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "FlyffPawn.generated.h"

// Forward decls
struct FInputActionValue;
class UCapsuleComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UFloatingPawnMovement;
class UInputMappingContext;
class UInputAction;

UCLASS()
class FLYFF_API AFlyffPawn : public APawn
{
	GENERATED_BODY()

public:
	AFlyffPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// Components
	UPROPERTY(VisibleAnywhere) UCapsuleComponent* Capsule;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere) UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere) UFloatingPawnMovement* Movement;

	// Enhanced Input (set in Details)
	UPROPERTY(EditAnywhere, Category = "Input") class UInputMappingContext* IMC_Default;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* IA_MoveForward;
	UPROPERTY(EditAnywhere, Category = "Input") class UInputAction* IA_MoveRight;

	float MoveForwardValue = 0.f;
	float MoveRightValue = 0.f;

	// Handlers
	void MoveForward(const struct FInputActionValue& Value);
	void MoveRight(const struct FInputActionValue& Value);


};