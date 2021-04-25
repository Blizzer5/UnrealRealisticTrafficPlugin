// Fill out your copyright notice in the Description page of Project Settings.


#include "URTVehicleStreet.h"
#include "URTStreet.h"

float AURTVehicleStreet::GetMaxSpeed() const
{
	if (IsValidPath())
	{
		return StreetMaxSpeed;
	}

	return 0.f;
}

void AURTVehicleStreet::GetAvailableStreets(TArray<AURTVehicleStreet*>& StreetsArray)
{
	StreetsArray.Append(LeftTurnStreets);
	StreetsArray.Append(ForwardStreets);
	StreetsArray.Append(RightTurnStreets);
}

void AURTVehicleStreet::BeginPlay()
{
	Super::BeginPlay();

	// km/h to cm/s
	StreetMaxSpeed *= 27.77f;

	Connections.Append(LeftTurnStreets);
	Connections.Append(ForwardStreets);
	Connections.Append(RightTurnStreets);
}

void AURTVehicleStreet::CreateLeftTurnStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTVehicleStreet* newLeftTurnStreet = GetWorld()->SpawnActor<AURTVehicleStreet>(AURTVehicleStreet::StaticClass(), SpawnInfo);
	if (newLeftTurnStreet)
	{
		newLeftTurnStreet->SetActorLocation(spawnLocation);
		newLeftTurnStreet->StreetSpline->AdaptLeftTurnForm();
		LeftTurnStreets.Add(newLeftTurnStreet);
	}
}

void AURTVehicleStreet::CreateForwardStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTVehicleStreet* newForwardStreet = GetWorld()->SpawnActor<AURTVehicleStreet>(AURTVehicleStreet::StaticClass(), SpawnInfo);
	if (newForwardStreet)
	{
		newForwardStreet->SetActorLocation(spawnLocation);
		newForwardStreet->StreetSpline->AdaptForwardForm();
		ForwardStreets.Add(newForwardStreet);
	}
}

void AURTVehicleStreet::CreateRightStreet()
{
	FVector spawnLocation = StreetSpline->GetWorldLocationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1);
	spawnLocation += StreetSpline->GetRotationAtSplinePoint(StreetSpline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World).Vector().ForwardVector * 400;
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AURTVehicleStreet* newRightTurnStreet = GetWorld()->SpawnActor<AURTVehicleStreet>(AURTVehicleStreet::StaticClass(), SpawnInfo);
	if (newRightTurnStreet)
	{
		newRightTurnStreet->SetActorLocation(spawnLocation);
		newRightTurnStreet->StreetSpline->AdaptRightTurnForm();
		RightTurnStreets.Add(newRightTurnStreet);
	}
}
