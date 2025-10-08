// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloatingActor.generated.h"

// Forward declaration for the mesh component
class UStaticMeshComponent;

UCLASS()
class FLYFF_API AFloatingActor : public AActor
{
	GENERATED_BODY()

public:
	AFloatingActor();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

private:
	// Visible mesh so the actor shows up in the world
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

	// Floating settings
	UPROPERTY(EditAnywhere, Category = "Floating")
	float Amplitude = 50.f;   // height of the bob (cm)

	UPROPERTY(EditAnywhere, Category = "Floating")
	float Speed = 1.5f;       // cycles per second

	UPROPERTY(VisibleAnywhere, Category = "Floating")
	FVector StartLocation;

	float RunningTime = 0.f;
};
