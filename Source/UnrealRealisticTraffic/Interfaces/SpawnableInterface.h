// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "../Street/URTStreet.h"
#include "SpawnableInterface.generated.h"

UENUM(BlueprintType)
enum class ESpawnableType : uint8
{
	Vehicle,
	Person,
};

UINTERFACE(MinimalAPI)
class USpawnableInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 *
 */
class UNREALREALISTICTRAFFIC_API ISpawnableInterface
{
	GENERATED_BODY()
public:
	ISpawnableInterface();

	virtual void OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned) { SetCurrentStreet(StreetToAppear, currentDistance); };
	virtual void OnActorCollected() { SetCurrentStreet(nullptr, -1); };
	virtual ESpawnableType GetType() { return Type; };
	virtual AURTStreet* GetCurrentStreet() { return CurrentStreet; };
	virtual void SetCurrentStreet(AURTStreet* newStreet, float distance) { CurrentStreet = newStreet; SetCurrentDistanceOnTheRoad(distance); };
	virtual float GetCurrentDistanceOnTheRoad() { return CurrentDistanceOnTheRoad; };
	virtual void SetCurrentDistanceOnTheRoad(float newDistanceOnTheRoad) { CurrentDistanceOnTheRoad = newDistanceOnTheRoad; };
	virtual bool IsActive() { return bIsActive; };

protected:
	ESpawnableType Type;
	AURTStreet* CurrentStreet;
	float CurrentDistanceOnTheRoad;
	bool bIsActive;
};
