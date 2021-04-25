// Fill out your copyright notice in the Description page of Project Settings.


#include "URTPersonBrain.h"
#include "../Street/URTStreet.h"
#include "../Controllers/URTBasePedestrianAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../Actors/URTBaseAIPerson.h"
#include "../Actors/URTTrafficLight.h"
#include "../Street/URTPedestrianStreet.h"
#include "../Actors/URTTrafficManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UURTPersonBrain::UURTPersonBrain()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UURTPersonBrain::BeginPlay()
{
	Super::BeginPlay();

	OwnerAsPerson = Cast<AURTBaseAIPerson>(GetOwner());

	OwnerController = Cast<AURTBasePedestrianAIController>(OwnerAsPerson->GetController());
	OwnerController->ReceiveMoveCompleted.AddDynamic(this, &UURTPersonBrain::OnMoveToCompleted);

	OwnerMovementComponent = OwnerAsPerson->FindComponentByClass<UCharacterMovementComponent>();
}


// Called every frame
void UURTPersonBrain::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentPedestrianState)
	{
	case EPedestrianMovementState::Stopped:
		break;
	case EPedestrianMovementState::Walking:
	{
		//TODO: THIS SHOULD BE DONE IN A TASK
		if (OwnerAsPerson->GetCurrentStreet()) {
			float newDistance = OwnerAsPerson->GetCurrentStreet()->ComputeNearestDistanceToLocation(OwnerAsPerson->GetActorLocation());
			OwnerAsPerson->SetCurrentDistanceOnTheRoad(newDistance);

			FVector Location;
			FRotator Rotator;
			float Distance = FMath::Clamp(OwnerAsPerson->GetDistanceToMoveTo(), 0.f, OwnerAsPerson->GetCurrentStreet()->GetLength());
			OwnerAsPerson->GetCurrentStreet()->GetLocationAndRotationAt(Distance, Location, Rotator);
			OwnerController->SetLocationToMoveTo(Location);
		}
	}
		break;
	case EPedestrianMovementState::RunningFrom:
		break;
	case EPedestrianMovementState::Seated:
		break;
	case EPedestrianMovementState::Swimming:
		break;
	case EPedestrianMovementState::Driving:
		break;

	}
}

void UURTPersonBrain::UpdateMoveVariables(FVector moveToLocation, float speed)
{
	if (OwnerController) {
		OwnerMovementComponent->MaxWalkSpeed = speed;
		OwnerController->SetLocationToMoveTo(moveToLocation);
	}
}

void UURTPersonBrain::OnMoveToCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (Result == EPathFollowingResult::Success) {
		switch (CurrentPedestrianState)
		{
		default:
			break;
		case EPedestrianMovementState::Stopped:
			break;
		case EPedestrianMovementState::Walking:
		{
			if (OwnerAsPerson->GetCurrentStreet() && CurrentPedestrianState >= EPedestrianMovementState::Stopped) {
				if (OwnerAsPerson->GetCurrentStreet()->GetCurrentStreetTrafficLightStateForPedestrians() != ELightState::Green) {
					OwnerAsPerson->GetCurrentStreet()->TrafficLight->OnPedestrianLightTurnedGreen.AddDynamic(this, &UURTPersonBrain::OnPedestrianLightTurnedGreen);
				}
				else
				{
					SetNextStreetToMoveTo();
				}
			}
		}
			break;
		case EPedestrianMovementState::RunningFrom:
		{
			SetPedestrianState(EPedestrianMovementState::Walking);
			TArray<AActor*> TrafficManagers;
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), AURTTrafficManager::StaticClass(), TrafficManagers);
			if (TrafficManagers.Num()) {
				AURTTrafficManager* TrafficManager = Cast<AURTTrafficManager>(TrafficManagers[0]);

				float newDistance;
				TrafficManager->GetNearestPedestrianStreetFromLocation(OwnerAsPerson->GetActorLocation(), newDistance);
				OwnerAsPerson->SetCurrentStreet(TrafficManager->GetNearestPedestrianStreetFromLocation(OwnerAsPerson->GetActorLocation(), newDistance), newDistance);
			}
		}
			break;
		case EPedestrianMovementState::Seated:
			break;
		case EPedestrianMovementState::Swimming:
			break;
		case EPedestrianMovementState::Driving:
			break;
		}
	}
	else
	{
		CurrentPedestrianState = EPedestrianMovementState::Stopped;
	}
}

void UURTPersonBrain::OnPedestrianLightTurnedGreen()
{
	OwnerAsPerson->GetCurrentStreet()->TrafficLight->OnPedestrianLightTurnedGreen.RemoveDynamic(this, &UURTPersonBrain::OnPedestrianLightTurnedGreen);
	SetNextStreetToMoveTo();
}

void UURTPersonBrain::SetNextStreetToMoveTo()
{	
	AURTPedestrianStreet* PedestrianStreet = Cast<AURTPedestrianStreet>(OwnerAsPerson->GetCurrentStreet());
	AURTStreet* newStreetToUse = StreetUsage == EStreetUsage::Forward ? PedestrianStreet->GetRandomForwardConnection() : PedestrianStreet->GetRandomBehindConnection();
	if (newStreetToUse) {
		OwnerAsPerson->SetCurrentStreet(newStreetToUse, StreetUsage == EStreetUsage::Forward ? 0.f : newStreetToUse->GetLength());
	}
	// If the street we're in doesn't have streets to go to, just turn back
	else {
		SetStreetUsage(GetStreetUsage() == EStreetUsage::Forward ? EStreetUsage::Reverse : EStreetUsage::Forward);
	}
}

EPedestrianMovementState UURTPersonBrain::GetPedestrianState()
{
	return CurrentPedestrianState;
}

void UURTPersonBrain::SetPedestrianState(EPedestrianMovementState newState)
{
	CurrentPedestrianState = newState;
}

EPedestrianObjective UURTPersonBrain::GetPedestrianCurrentObjective()
{
	return PedestrianObjective;
}

void UURTPersonBrain::SetPedestrianObjective(EPedestrianObjective newObjective)
{
	PedestrianObjective = newObjective;
}

EStreetUsage UURTPersonBrain::GetStreetUsage()
{
	return StreetUsage;
}

void UURTPersonBrain::SetStreetUsage(EStreetUsage newStreetUsage)
{
	StreetUsage = newStreetUsage;
}

void UURTPersonBrain::Reset()
{
	CurrentPedestrianState = EPedestrianMovementState::Stopped;
}

void UURTPersonBrain::OnGotScared(AActor* ScaredBy)
{
	SetPedestrianObjective(EPedestrianObjective::RunFrom);
	OwnerController->SetRunningFrom(ScaredBy);
}

#if WITH_EDITOR
void UURTPersonBrain::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UURTPersonBrain, PedestrianObjective)) {
		switch (PedestrianObjective)
		{
		default:
			break;
		case EPedestrianObjective::None:
			break;
		case EPedestrianObjective::Walk:
			break;
		case EPedestrianObjective::RunFrom:
			OnGotScared(GetWorld()->GetFirstPlayerController()->GetPawn());
			break;
		case EPedestrianObjective::ToSeat:
			break;
		case EPedestrianObjective::ToSwim:
			break;
		case EPedestrianObjective::ToDrive:
			break;
		}
	}
}
#endif
