// Fill out your copyright notice in the Description page of Project Settings.


#include "URTTrafficManager.h"
#include "URTBaseVehicle.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "../Interfaces/SpawnableInterface.h"
#include "../Street/URTPedestrianStreet.h"
#include "../Controllers/URTBasePedestrianAIController.h"

// Sets default values
AURTTrafficManager::AURTTrafficManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AURTTrafficManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		check(CarsClassToSpawn.Num() > 0 && TEXT("No car classes to spawn."));
		check(PersonClassToSpawn.Num() > 0 && TEXT("No person classes to spawn."));

		TArray<AActor*> Streets;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AURTVehicleStreet::StaticClass(), Streets);
		for (AActor* street : Streets)
		{
			WorldCarStreets.Add(Cast<AURTVehicleStreet>(street));
		}
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AURTPedestrianStreet::StaticClass(), Streets);
		for (AActor* street : Streets)
		{
			WorldPedestrianStreets.Add(Cast<AURTPedestrianStreet>(street));
		}

		CreateActors();
	}
	else
	{
		SetActorTickEnabled(false);
	}
}

// Called every frame
void AURTTrafficManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	TryToPutActorsInScene();
	DespawnActors();
}

AURTPedestrianStreet* AURTTrafficManager::GetNearestPedestrianStreetFromLocation(FVector Location, float& distanceToGo)
{
	float LocationsDistance = 999999;
	AURTPedestrianStreet* nearestStreet = nullptr;
	for (AURTPedestrianStreet* street : WorldPedestrianStreets)	{
		float distanceBetween = FVector::Dist2D(Location, street->GetActorLocation());
		if (distanceBetween < LocationsDistance) {
			LocationsDistance = distanceBetween;
			nearestStreet = street;
		}
	}

	if (nearestStreet) {
		distanceToGo = nearestStreet->ComputeNearestDistanceToLocation(Location);
	}

	return nearestStreet;
}

void AURTTrafficManager::CreateActors()
{
	if (UWorld* TMWorld = GetWorld())
	{
		// Vehicles
		for (int i = 0; i < CarsClassToSpawn.Num(); ++i)
		{
			int index = 0;
			while (index < numOfCarsToSpawn)
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AURTBaseVehicle* newVehicle = Cast<AURTBaseVehicle>(TMWorld->SpawnActor<AURTBaseVehicle>(CarsClassToSpawn[i]->GetDefaultObject()->GetClass(), SpawnInfo));
				if (newVehicle)
				{
					newVehicle->GetMesh()->SetSimulatePhysics(false);
					newVehicle->SetActorHiddenInGame(true);
					CarPool.Add(newVehicle);
				}
				index++;
			}
		}

		// Persons
		for (int i = 0; i < PersonClassToSpawn.Num(); ++i)
		{
			int index = 0;
			while (index < numOfPersonsToSpawn)
			{
				FActorSpawnParameters SpawnInfo;
				SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
				AURTBaseAIPerson* newPerson = Cast<AURTBaseAIPerson>(TMWorld->SpawnActor<AURTBaseAIPerson>(PersonClassToSpawn[i]->GetDefaultObject()->GetClass(), SpawnInfo));
				if (newPerson)
				{
					newPerson->GetMesh()->SetSimulatePhysics(false);
					newPerson->SetActorHiddenInGame(true);
					PersonsPool.Add(newPerson);
				}
				index++;
			}
		}
	}
}

void AURTTrafficManager::TryToPutActorsInScene()
{
	if (UWorld* TMWorld = GetWorld())
	{
		// We need the players locations so we can spawn actors without players seeing it
		FConstPlayerControllerIterator GamePlayerControllers = TMWorld->GetPlayerControllerIterator();
		TArray<FVector> PlayersLocations;
		for (GamePlayerControllers; GamePlayerControllers; ++GamePlayerControllers)
		{
			APlayerController* GamePlayerController = GamePlayerControllers->Get();
			if (GamePlayerController->GetPawn())
			{
				PlayersLocations.Add(GamePlayerController->GetPawn()->GetActorLocation());
			}
		}

		if (PlayersLocations.Num() <= 0)
		{
			return;
		}

		// Vehicles spawn
		for (AURTVehicleStreet* Street : WorldCarStreets)
		{
			if (Street && numOfCarsInScene < maxNumCarsInScene)
			{
				FVector Location;
				FRotator Rotator;
				Street->GetLocationAndRotationAt(0.f, Location, Rotator);
				// Just so we don't hit and spawn on the floor
				Location.Z += 20;
				if (Street->CanSpawnActorOnDistance())
				{
					for (int j = 0; j < PlayersLocations.Num(); j++)
					{
						if (CanSpawnOnStreetFromPlayerLocation(PlayersLocations[j], Location) && IsLocationNotVisible(PlayersLocations[j], Location))
						{
							AURTBaseVehicle* VehicleToSpawn;

							if (CarPool.Num() <= 0)
							{
								FActorSpawnParameters SpawnInfo;
								SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
								VehicleToSpawn = Cast<AURTBaseVehicle>(TMWorld->SpawnActor<AURTBaseVehicle>(CarsClassToSpawn[FMath::RandRange(0, CarsClassToSpawn.Num() - 1)]->GetDefaultObject()->GetClass(), SpawnInfo));
							}
							else
							{
								VehicleToSpawn = Cast<AURTBaseVehicle>(CarPool[FMath::RandRange(0, CarPool.Num() - 1)]);
								CarPool.Remove(VehicleToSpawn);
							}

							VehicleToSpawn->OnActorSpawnedInScene(Street, 0.f, Location, Rotator);

							ActorsInScene.AddUnique(VehicleToSpawn);

							numOfCarsInScene++;
						}
					}
				}
			}
		}

		// Pedestrian spawns
		for (AURTPedestrianStreet* Street : WorldPedestrianStreets)
		{
			if (Street && numOfPersonsInScene < maxNumPersonsInScene && Street->GetNumActorsOnThisStreet() < Street->MaxPedestrians)
			{
				FVector Location;
				FRotator Rotator;
				float distanceToSpawn = FMath::RandRange(0.f, Street->GetLength());
				Street->GetLocationAndRotationAt(distanceToSpawn, Location, Rotator);
				// Just so we don't hit and spawn on the floor
				Location.Z += 200;
				if (Street->CanSpawnActorOnDistance())
				{
					for (int j = 0; j < PlayersLocations.Num(); j++)
					{
						if (CanSpawnOnStreetFromPlayerLocation(PlayersLocations[j], Location) && IsLocationNotVisible(PlayersLocations[j], Location))
						{
							AURTBaseAIPerson* PedestrianToSpawn;

							if (PersonsPool.Num() <= 0)
							{
								FActorSpawnParameters SpawnInfo;
								SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
								PedestrianToSpawn = Cast<AURTBaseAIPerson>(TMWorld->SpawnActor<AURTBaseAIPerson>(PersonClassToSpawn[FMath::RandRange(0, PersonClassToSpawn.Num() - 1)]->GetDefaultObject()->GetClass(), SpawnInfo));
							}
							else
							{
								PedestrianToSpawn = Cast<AURTBaseAIPerson>(PersonsPool[FMath::RandRange(0, PersonsPool.Num() - 1)]);
								PersonsPool.Remove(PedestrianToSpawn);
							}

							PedestrianToSpawn->GetPersonBrain()->SetStreetUsage(FMath::FRandRange(0.f, 1.f) <= 0.5f ? EStreetUsage::Forward : EStreetUsage::Reverse);

							PedestrianToSpawn->OnActorSpawnedInScene(Street, distanceToSpawn, Location, Rotator);

							ActorsInScene.AddUnique(PedestrianToSpawn);

							numOfPersonsInScene++;
						}
					}
				}
			}
		}
	}
}

void AURTTrafficManager::DespawnActors(bool bForce)
{
	if (UWorld* TMWorld = GetWorld())
	{
		// We need the players locations so we can spawn actors without players seeing it
		FConstPlayerControllerIterator GamePlayerControllers = TMWorld->GetPlayerControllerIterator();
		TArray<FVector> PlayersLocations;
		for (GamePlayerControllers; GamePlayerControllers; ++GamePlayerControllers)
		{
			APlayerController* GamePlayerController = GamePlayerControllers->Get();
			if (GamePlayerController->GetPawn())
			{
				PlayersLocations.Add(GamePlayerController->GetPawn()->GetActorLocation());
			}
		}

		TArray<AActor*> ActorsToRemove;

		for (int i = 0; i < ActorsInScene.Num(); i++)
		{
			for (int j = 0; j < PlayersLocations.Num(); j++)
			{
				if (bForce || (FVector::Dist2D(ActorsInScene[i]->GetActorLocation(), PlayersLocations[j]) > DistanceToDespawn&& IsLocationNotVisible(PlayersLocations[j], ActorsInScene[i]->GetActorLocation())))
				{
					AActor* actorToRemove = ActorsInScene[i];
					if (ISpawnableInterface* InterfaceActor = Cast<ISpawnableInterface>(actorToRemove))
					{
						ActorsToRemove.Add(actorToRemove);
						
						InterfaceActor->OnActorCollected();

						if (InterfaceActor->GetType() == ESpawnableType::Vehicle)
						{
							CarPool.AddUnique(Cast<AURTBaseVehicle>(actorToRemove));
							numOfCarsInScene--;
						}
						else if (InterfaceActor->GetType() == ESpawnableType::Person)
						{
							PersonsPool.AddUnique(Cast<AURTBaseAIPerson>(actorToRemove));
							numOfPersonsInScene--;
						}
					}
				}
			}
		}

		ActorsInScene.RemoveAll([=](const AActor* SpawnableActor) {
			return ActorsToRemove.Contains(SpawnableActor);
		});
	}
}

bool AURTTrafficManager::IsLocationNotVisible(FVector origin, FVector destination)
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Append(ActorsInScene);
	TArray<FHitResult> Hits;
	UKismetSystemLibrary::LineTraceMulti(this, origin, destination, UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Pawn), false, ActorsToIgnore, EDrawDebugTrace::None, Hits, true);

	return Hits.Num() > 0;
}

bool AURTTrafficManager::CanSpawnOnStreetFromPlayerLocation(FVector Playerlocation, FVector StreetLocation)
{
	float Distance = FVector::Dist(Playerlocation, StreetLocation);
	if (Distance > DistanceToSpawn&& Distance < DistanceToDespawn)
	{
		return true;
	}
	return false;
}

void AURTTrafficManager::ClearRoads()
{
	DespawnActors(true);
}

