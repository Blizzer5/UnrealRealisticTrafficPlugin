// Fill out your copyright notice in the Description page of Project Settings.


#include "URTStreet.h"
#include "Components/SplineComponent.h"

// Sets default values
AURTStreet::AURTStreet()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	StreetSpline = CreateDefaultSubobject<UURTStreetSplineComponent>("Street Spline");
}

// Called when the game starts or when spawned
void AURTStreet::BeginPlay()
{
	Super::BeginPlay();
}

float AURTStreet::GetLength() const
{
	if (IsValidPath())
	{
		return StreetSpline->GetSplineLength();
	}

	return 0.f;
}

const FVector AURTStreet::GetStartLocation() const
{
	if (IsValidPath())
	{
		return StreetSpline->GetLocationAtDistanceAlongSpline(0.0f, ESplineCoordinateSpace::World);
	}
	return FVector::ZeroVector;
}

const FVector AURTStreet::GetEndLocation() const
{
	if (IsValidPath())
	{
		return StreetSpline->GetLocationAtDistanceAlongSpline(GetLength(), ESplineCoordinateSpace::World);
	}
	return FVector::ZeroVector;
}

const FVector AURTStreet::GetMiddleLocation() const
{
	if (IsValidPath())
	{
		return StreetSpline->GetLocationAtDistanceAlongSpline(GetLength() / 2.f, ESplineCoordinateSpace::World);
	}
	return FVector::ZeroVector;
}

void AURTStreet::GetLocationAndRotationAt(float InDistance, FVector& OutLocation, FRotator& OutRotation) const
{
	if (!IsValidPath())
	{
		return;
	}

	const float Param = StreetSpline->SplineCurves.ReparamTable.Eval(InDistance, 0.0f);
	OutLocation = StreetSpline->GetLocationAtSplineInputKey(Param, ESplineCoordinateSpace::World);
	OutRotation = StreetSpline->GetRotationAtSplineInputKey(Param, ESplineCoordinateSpace::World);
}

float AURTStreet::ComputeNearestDistanceToLocation(FVector InLoc) const
{
	if (!IsValidPath())
	{
		return 0.f;
	}

	const float ClosestInputKey = StreetSpline->FindInputKeyClosestToWorldLocation(InLoc);
	const int32 PreviousPoint = FMath::TruncToInt(ClosestInputKey);

	// Lerp between the previous and the next spline points
	float Distance = StreetSpline->GetDistanceAlongSplineAtSplinePoint(PreviousPoint);
	Distance += (ClosestInputKey - PreviousPoint) * (StreetSpline->GetDistanceAlongSplineAtSplinePoint(PreviousPoint + 1) - Distance);

	// The linear approximation is not enough
	// So here is a kinda numerical approximation, a couple of iterations should do it
	for (int32 i = 0; i < 2; ++i)
	{
		const float InputKeyAtDistance = StreetSpline->SplineCurves.ReparamTable.Eval(Distance, 0.0f);

		// The euclidean distance between the current calculated distance and the real closest point
		const float Delta = (StreetSpline->GetLocationAtSplineInputKey(InputKeyAtDistance, ESplineCoordinateSpace::World) - StreetSpline->GetLocationAtSplineInputKey(ClosestInputKey, ESplineCoordinateSpace::World)).Size();

		if (InputKeyAtDistance < ClosestInputKey)
		{
			Distance += Delta;
		}
		else if (InputKeyAtDistance > ClosestInputKey)
		{
			Distance -= Delta;
		}
		else
		{
			break;
		}
	}

	return FMath::Clamp(Distance, 0.0f, GetLength());
}

float AURTStreet::ComputeDistanceWith(float InCurDistance, float InAddedDistance, bool bInLoop /*= true*/) const
{
	const float Length = GetLength();
	float NewDist = InCurDistance + InAddedDistance;

	if (NewDist > Length)
	{
		if (bInLoop)
			NewDist = NewDist - Length;
		else
			NewDist = Length;
	}
	else if (NewDist < 0.0f)
	{
		if (bInLoop)
			NewDist = NewDist + Length;
		else
			NewDist = 0.0f;
	}

	return NewDist;
}

bool AURTStreet::IsValidPath() const
{
	return StreetSpline != nullptr;
}

bool AURTStreet::CanSpawnActorOnDistance(float Distance)
{
	if (StreetSpline)
	{
		FVector DistanceLocation = StreetSpline->GetLocationAtDistanceAlongSpline(Distance, ESplineCoordinateSpace::World);
		for (uint8 i = 0; i < ActorsOnThisStreet.Num(); ++i)
		{
			if (FMath::Abs(ComputeNearestDistanceToLocation(ActorsOnThisStreet[i]->GetActorLocation()) - Distance) < distanceToSpawn)
			{
				return false;
			}
		}
		return true;
	}
	return false;
}

void AURTStreet::AddActorOnThisStreet(AActor* ActorToAdd)
{
	ActorsOnThisStreet.AddUnique(ActorToAdd);
}

void AURTStreet::RemoveActorOnThisStreet(AActor* ActorToAdd)
{
	ActorsOnThisStreet.Remove(ActorToAdd);
}

bool AURTStreet::IsStreetAConnection(AURTStreet* StreetToCheck)
{
	return Connections.Contains(StreetToCheck);
}

ELightState AURTStreet::GetCurrentStreetTrafficLightStateForVehicles()
{
	return TrafficLight ? TrafficLight->GetCurrentLightStateForVehicles() : ELightState::Green;
}

ELightState AURTStreet::GetCurrentStreetTrafficLightStateForPedestrians()
{
	return TrafficLight ? TrafficLight->GetCurrentLightStateForPedestrians() : ELightState::Green;
}

AURTStreet* AURTStreet::GetRandomConnection()
{
	return Connections.Num() > 0 ? Connections[FMath::RandRange(0, Connections.Num() - 1)] : nullptr;
}

int AURTStreet::GetNumActorsOnThisStreet()
{
	return ActorsOnThisStreet.Num();
}
