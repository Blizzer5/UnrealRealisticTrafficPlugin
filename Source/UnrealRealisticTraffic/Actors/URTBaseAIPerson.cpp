// Fill out your copyright notice in the Description page of Project Settings.


#include "URTBaseAIPerson.h"
#include "NavigationInvokerComponent.h"
#include "../Controllers/URTBasePedestrianAIController.h"

AURTBaseAIPerson::AURTBaseAIPerson()
{
	PrimaryActorTick.bCanEverTick = false;

	Type = ESpawnableType::Person;

	PersonBrain = CreateDefaultSubobject<UURTPersonBrain>(TEXT("Brain"));

	NavigationInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavigationInvoker"));
	NavigationInvoker->bAutoActivate = false;
}

UURTPersonBrain* AURTBaseAIPerson::GetPersonBrain()
{
	return PersonBrain;
}

void AURTBaseAIPerson::OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned)
{
	Super::OnActorSpawnedInScene(StreetToAppear, currentDistance, LocationSpawned, RotationSpawned);
	NavigationInvoker->Activate(true);
	bIsActive = true;
	PersonController->StartBehaviorTree();
}

void AURTBaseAIPerson::OnActorCollected()
{
	Super::OnActorCollected();
	NavigationInvoker->Deactivate();
	bIsActive = false;
	PersonController->StopBehaviorTree();
}

void AURTBaseAIPerson::SetCurrentStreet(AURTStreet* newStreet, float distance)
{
	Super::SetCurrentStreet(newStreet, distance);
	if (newStreet) {
		FVector Location;
		FRotator Rotator;
		float newDistance = distance;
		// We need to recheck the street usage
		if (distance == 0.f || distance == GetCurrentStreet()->GetLength()) {
			newDistance = GetCurrentStreet()->ComputeNearestDistanceToLocation(GetActorLocation());
			if (newDistance != distance) {
				if (newDistance == 0.f) {
					PersonBrain->SetStreetUsage(EStreetUsage::Forward);
				}
				else if (newDistance == GetCurrentStreet()->GetLength())
				{
					PersonBrain->SetStreetUsage(EStreetUsage::Reverse);
				}
			}
		}
		distance = newDistance;

		float DistanceToMoveTo = FMath::Clamp(GetDistanceToMoveTo(), 0.f, GetCurrentStreet()->GetLength());
		
		GetCurrentStreet()->GetLocationAndRotationAt(distance, Location, Rotator);
		PersonController->SetLocationToMoveTo(Location);
	}
}

float AURTBaseAIPerson::GetDistanceToMoveTo()
{
	return GetCurrentDistanceOnTheRoad() + (PersonBrain->GetStreetUsage() == EStreetUsage::Forward ? 300.0f : -300.f);
}

void AURTBaseAIPerson::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AURTBaseAIPerson::BeginPlay()
{
	Super::BeginPlay();

	PersonController = Cast<AURTBasePedestrianAIController>(GetController());
}
