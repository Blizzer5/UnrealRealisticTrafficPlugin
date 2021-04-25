// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Components/URTStreetSplineComponent.h"
#include "../Actors/URTTrafficLight.h"
#include "URTStreet.generated.h"

UCLASS()
class UNREALREALISTICTRAFFIC_API AURTStreet : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AURTStreet();

	/** Street spline */
	UPROPERTY(Category = Street, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UURTStreetSplineComponent* StreetSpline;

	/* Traffic light associated with this street */
	UPROPERTY(Category = "Street | TrafficLight", EditAnywhere, meta = (AllowPrivateAccess = "true"))
		AURTTrafficLight* TrafficLight;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	/** Return the length of this path */
	UFUNCTION(BlueprintPure, Category = Path)
		float GetLength() const;

	/** Return the start location of the path */
	UFUNCTION(BlueprintPure, Category = Path)
		const FVector GetStartLocation() const;

	/** Return the middle location of the path */
	UFUNCTION(BlueprintPure, Category = Path)
		const FVector GetMiddleLocation() const;

	/** Return the end location of the path */
	UFUNCTION(BlueprintPure, Category = Path)
		const FVector GetEndLocation() const;

	/** Return location and rotation of this road at given distance along it */
	UFUNCTION(BlueprintPure, Category = Path)
		void GetLocationAndRotationAt(float InDistance, FVector& OutLocation, FRotator& OutRotation) const;

	/**
	* Computes at what distance in the road given location is nearest to
	* @param InLoc The location to test where it's nearest in road
	* @return The distance in our road which is nearest to given location
	*/
	UFUNCTION(BlueprintPure, Category = Path)
		float ComputeNearestDistanceToLocation(FVector InLoc) const;

	/**
	* Computes the final distance in curve given the addition (will be clamped between 0 and road length)
	* @param InCurDistance The distance in the curve to start with
	* @param InAddedDistance The distance to add, for example, if road length is 100, and we pass 90 as InCurDistance and 110 as InAddedDistance, it will result in 10
	* @param bInReverse Whether we are going backwards on the road
	* @return The final distance
	*/
	UFUNCTION(BlueprintPure, Category = Path)
		float ComputeDistanceWith(float InCurDistance, float InAddedDistance, bool bInLoop = true) const;

	/** True if this path is valid */
	UFUNCTION(BlueprintPure, Category = Path)
		bool IsValidPath() const;

	/* Returns if we can spawn an actor depending on where we want to spawn it */
	virtual bool CanSpawnActorOnDistance(float Distance = 0);

	void AddActorOnThisStreet(AActor* ActorToAdd);
	void RemoveActorOnThisStreet(AActor* ActorToAdd);

	bool IsStreetAConnection(AURTStreet* StreetToCheck);

	ELightState GetCurrentStreetTrafficLightStateForVehicles();
	ELightState GetCurrentStreetTrafficLightStateForPedestrians();

	AURTStreet* GetRandomConnection();
	int GetNumActorsOnThisStreet();

protected:
	TArray<AActor*> ActorsOnThisStreet;
	TArray<AURTStreet*> Connections;
	int distanceToSpawn = 2500;
};
