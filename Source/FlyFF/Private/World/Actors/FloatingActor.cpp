// Fill out your copyright notice in the Description page of Project Settings.


#include "World/Actors/FloatingActor.h"
#include "Engine/Engine.h"
#include "Components/StaticMeshComponent.h"

AFloatingActor::AFloatingActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create a visible mesh for the actor
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh; // make the mesh the root of the actor
}

void AFloatingActor::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = GetActorLocation();
	UE_LOG(LogTemp, Log, TEXT("FloatingActor spawned at: %s"), *StartLocation.ToString());
}

void AFloatingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RunningTime += DeltaTime;
	const float OffsetZ = Amplitude * FMath::Sin(2.f * PI * Speed * RunningTime);
	SetActorLocation(StartLocation + FVector(0, 0, OffsetZ));
}
