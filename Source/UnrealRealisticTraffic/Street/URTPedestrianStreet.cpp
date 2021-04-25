// Fill out your copyright notice in the Description page of Project Settings.


#include "URTPedestrianStreet.h"
#include "URTStreet.h"

AURTPedestrianStreet::AURTPedestrianStreet()
{
	distanceToSpawn = 250;
}

AURTStreet* AURTPedestrianStreet::GetRandomForwardConnection()
{
	return ForwardPathsToGoTo.Num() > 0 ? ForwardPathsToGoTo[FMath::RandRange(0, ForwardPathsToGoTo.Num() - 1)] : nullptr;
}

AURTStreet* AURTPedestrianStreet::GetRandomBehindConnection()
{
	return BehindPathsToGoTo.Num() > 0 ? BehindPathsToGoTo[FMath::RandRange(0, BehindPathsToGoTo.Num() - 1)] : nullptr;
}

#if WITH_EDITOR
void AURTPedestrianStreet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AURTPedestrianStreet, ForwardPathsToGoTo)) {
		if (ForwardPathsToGoTo.Num() > 0 && ForwardPathsToGoTo.Last() != nullptr)
		{
			ForwardPathsToGoTo.Last()->BehindPathsToGoTo.AddUnique(this);
		}
	}
}
#endif

void AURTPedestrianStreet::GetAvailableStreets(TArray<AURTPedestrianStreet*>& StreetsArray)
{
	//TODO: Isto depende de se o actor vem a andar para a frente ou para trás na spline.
	// Checar isto
	StreetsArray.Append(ForwardPathsToGoTo);
}

void AURTPedestrianStreet::BeginPlay()
{
	Super::BeginPlay();

	Connections.Append(ForwardPathsToGoTo);
	Connections.Append(BehindPathsToGoTo);
}

void AURTPedestrianStreet::CreateLeftTurnStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTPedestrianStreet* newLeftTurnStreet = GetWorld()->SpawnActor<AURTPedestrianStreet>(AURTPedestrianStreet::StaticClass(), SpawnInfo);
	if (newLeftTurnStreet)
	{
		newLeftTurnStreet->SetActorLocation(spawnLocation);
		newLeftTurnStreet->StreetSpline->AdaptLeftTurnForm();
		ForwardPathsToGoTo.Add(newLeftTurnStreet);
	}
}

void AURTPedestrianStreet::CreateForwardStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTPedestrianStreet* newForwardStreet = GetWorld()->SpawnActor<AURTPedestrianStreet>(AURTPedestrianStreet::StaticClass(), SpawnInfo);
	if (newForwardStreet)
	{
		newForwardStreet->SetActorLocation(spawnLocation);
		newForwardStreet->StreetSpline->AdaptForwardForm();
		ForwardPathsToGoTo.Add(newForwardStreet);
	}
}

void AURTPedestrianStreet::CreateRightStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTPedestrianStreet* newRightTurnStreet = GetWorld()->SpawnActor<AURTPedestrianStreet>(AURTPedestrianStreet::StaticClass(), SpawnInfo);
	if (newRightTurnStreet)
	{
		newRightTurnStreet->SetActorLocation(spawnLocation);
		newRightTurnStreet->StreetSpline->AdaptRightTurnForm();
		ForwardPathsToGoTo.Add(newRightTurnStreet);
	}
}
