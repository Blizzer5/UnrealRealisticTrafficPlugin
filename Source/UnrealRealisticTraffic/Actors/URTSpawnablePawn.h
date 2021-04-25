// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Interfaces/SpawnableInterface.h"
#include "URTSpawnablePawn.generated.h"

UCLASS()
class UNREALREALISTICTRAFFIC_API AURTSpawnablePawn : public APawn, public ISpawnableInterface
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AURTSpawnablePawn(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

public:
	void OnActorSpawnedInScene(AURTStreet* StreetToAppear, float distanceSpawned, FVector LocationSpawned, FRotator RotationSpawned) override;
	void OnActorCollected() override;

};
