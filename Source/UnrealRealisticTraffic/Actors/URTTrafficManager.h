// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "URTBaseVehicle.h"
#include "URTBaseAIPerson.h"
#include "../Street/URTPedestrianStreet.h"
#include "../Street/URTVehicleStreet.h"
#include "URTTrafficManager.generated.h"

UCLASS()
class UNREALREALISTICTRAFFIC_API AURTTrafficManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AURTTrafficManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	AURTPedestrianStreet* GetNearestPedestrianStreetFromLocation(FVector Location, float& distanceToGo);
private:
	void CreateActors();
	void TryToPutActorsInScene();
	void DespawnActors(bool bForce = false);
	bool IsLocationNotVisible(FVector origin, FVector destination);
	bool CanSpawnOnStreetFromPlayerLocation(FVector Playerlocation, FVector StreetLocation);
public:
	// Max distance to check which actors should be spawned
	UPROPERTY(Category = TrafficManagerSpawn, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		float DistanceToSpawn = 20000;

	// Distance to check which actors should be despawned
	UPROPERTY(Category = TrafficManagerSpawn, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		float DistanceToDespawn = 30000;

	// The number of cars that can be in scene at the same time 
	UPROPERTY(Category = TrafficManagerCars, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		int maxNumCarsInScene = 20;

	// The number of cars we want to spawn for each car class so we can have them in a pool.
	// If maxNumCarsInScene > numOfCarsToSpawn then we will spawn an actor which may create a spike
	UPROPERTY(Category = TrafficManagerCars, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		int numOfCarsToSpawn = 20;

	// The number of cars that can be in scene at the same time 
	UPROPERTY(Category = TrafficManagerPersons, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		int maxNumPersonsInScene = 20;

	// The number of cars we want to spawn for each car class so we can have them in a pool.
	// If maxNumCarsInScene > numOfCarsToSpawn then we will spawn an actor which may create a spike
	UPROPERTY(Category = TrafficManagerPersons, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		int numOfPersonsToSpawn = 20;

	UPROPERTY(Category = TrafficManager, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AURTBaseVehicle>> CarsClassToSpawn;

	UPROPERTY(Category = TrafficManager, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		TArray<TSubclassOf<AURTBaseAIPerson>> PersonClassToSpawn;

private:
	TArray<AURTSpawnablePawn*> CarPool;
	TArray<AURTSpawnableCharacter*> PersonsPool;
	TArray<AActor*> ActorsInScene;
	TArray<AURTVehicleStreet*> WorldCarStreets;
	TArray<AURTPedestrianStreet*> WorldPedestrianStreets;
	int numOfCarsInScene = 0;
	int numOfPersonsInScene = 0;

	UFUNCTION(CallInEditor)
	void ClearRoads();
};
