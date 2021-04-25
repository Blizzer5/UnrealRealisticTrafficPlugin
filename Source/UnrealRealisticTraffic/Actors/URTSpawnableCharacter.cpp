// Fill out your copyright notice in the Description page of Project Settings.


#include "URTSpawnableCharacter.h"

// Sets default values
AURTSpawnableCharacter::AURTSpawnableCharacter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AURTSpawnableCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AURTSpawnableCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AURTSpawnableCharacter::OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned)
{
	SetCurrentStreet(StreetToAppear, currentDistance);
	CurrentStreet->AddActorOnThisStreet(this);
	SetActorLocation(LocationSpawned, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(RotationSpawned, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(false);
}

void AURTSpawnableCharacter::OnActorCollected()
{
	if(CurrentStreet) {
		CurrentStreet->RemoveActorOnThisStreet(this);
		SetCurrentStreet(nullptr, -1);
	}
	SetActorLocation(FVector::ZeroVector, false, nullptr, ETeleportType::TeleportPhysics);
	SetActorRotation(FRotator::ZeroRotator, ETeleportType::TeleportPhysics);
	SetActorHiddenInGame(true);
}

