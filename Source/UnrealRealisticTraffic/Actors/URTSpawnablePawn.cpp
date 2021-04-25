// Fill out your copyright notice in the Description page of Project Settings.


#include "URTSpawnablePawn.h"

// Sets default values
AURTSpawnablePawn::AURTSpawnablePawn(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AURTSpawnablePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AURTSpawnablePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AURTSpawnablePawn::OnActorSpawnedInScene(AURTStreet* StreetToAppear, float distanceSpawned, FVector LocationSpawned, FRotator RotationSpawned)
{
	SetCurrentStreet(StreetToAppear, distanceSpawned);
	CurrentStreet->AddActorOnThisStreet(this);
	SetActorLocation(LocationSpawned, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(RotationSpawned, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
}

void AURTSpawnablePawn::OnActorCollected()
{
	CurrentStreet->RemoveActorOnThisStreet(this);
	SetCurrentStreet(nullptr, -1);

	SetActorLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(FRotator::ZeroRotator, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(true);
}

