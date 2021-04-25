// Fill out your copyright notice in the Description page of Project Settings.


#include "URTVehicleDetection.h"
#include "../Actors/URTBaseVehicle.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/CollisionProfile.h"

// Sets default values for this component's properties
UURTVehicleDetection::UURTVehicleDetection()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UURTVehicleDetection::BeginPlay()
{
	Super::BeginPlay();

	OwnerVehicle = Cast<AURTBaseVehicle>(GetOwner());
}


// Called every frame
void UURTVehicleDetection::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DoTrace();
}

void UURTVehicleDetection::SetIsReversing(bool bIsVehicleReversing)
{
	bIsReversing = bIsVehicleReversing;
}

EDetectionType UURTVehicleDetection::GetCurrentDetection()
{
	return CurrentDetectionType;
}

void UURTVehicleDetection::DoTrace()
{
	FVector ForwardVector = bIsReversing ? GetOwner()->GetActorForwardVector() * -1 : GetOwner()->GetActorForwardVector();
	FVector LineStart = GetOwner()->GetActorLocation() + (DetectionCenterMultiplier * ForwardVector);
	LineStart.Z += DetectionCenterZOverride;
	ETraceTypeQuery TraceQuery = UEngineTypes::ConvertToTraceType(ECC_Vehicle);
	float VelocityMultiplier = FMath::Max(GetOwner()->GetVelocity().Size(), 200.f);

	/*// Left Trace
	FHitResult RightSideHitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		LineStart,
		LineStart + (GetOwner()->GetActorRightVector() * SideDetectionsSize),
		TraceQuery,
		true,
		{ GetOwner() },
		DebugTrace,
		RightSideHitResult,
		true
	);

	// Right Trace
	FHitResult LeftSideHitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		LineStart,
		LineStart + (-1 * GetOwner()->GetActorRightVector() * SideDetectionsSize),
		TraceQuery,
		true,
		{ GetOwner() },
		DebugTrace,
		LeftSideHitResult,
		true
	);*/

	
	//Forward or Backwards Trace
	FHitResult ForwardHitResult;
	UKismetSystemLibrary::LineTraceSingle(
		this,
		LineStart,
		LineStart + (ForwardVector * VelocityMultiplier * ForwardBackwardDetectionMultiplier),
		TraceQuery,
		true,
		{ GetOwner() },
		DebugTrace,
		ForwardHitResult,
		true
	);

	VelocityMultiplier = FMath::Max(GetOwner()->GetVelocity().Size(), 350.f);

	FHitResult LeftDiagonalHitResult;
	FVector LeftDiagonalDirection = UKismetMathLibrary::RotateAngleAxis(ForwardVector, -DiagonalAngle, FVector::UpVector);
	UKismetSystemLibrary::LineTraceSingle(
		this,
		LineStart,
		LineStart + (LeftDiagonalDirection * VelocityMultiplier * DiagonalDetectionsMultiplier),
		TraceQuery,
		true,
		{ GetOwner() },
		DebugTrace,
		LeftDiagonalHitResult,
		true
	);

	FHitResult RightDiagonalHitResult;
	FVector RightDiagonalDirection = UKismetMathLibrary::RotateAngleAxis(ForwardVector, DiagonalAngle, FVector::UpVector);
	UKismetSystemLibrary::LineTraceSingle(
		this,
		LineStart,
		LineStart + (RightDiagonalDirection * VelocityMultiplier * DiagonalDetectionsMultiplier),
		TraceQuery,
		true,
		{ GetOwner() },
		DebugTrace,
		RightDiagonalHitResult,
		true
	);

	bool bBlockedForward = IsActorBlockable(ForwardHitResult.GetActor());
	bool bBlockedLeft = IsActorBlockable(LeftDiagonalHitResult.GetActor());
	bool bBlockedRight = IsActorBlockable(RightDiagonalHitResult.GetActor());

	EDetectionType detectionMade = EDetectionType::None;
	if (bBlockedForward)
	{
		if ((bBlockedLeft && bBlockedRight) || (!bBlockedLeft && !bBlockedRight))
		{
			detectionMade = EDetectionType::Forward;
		}
		else if (bBlockedLeft)
		{
			detectionMade = EDetectionType::LeftForward;
		}
		else if (bBlockedRight)
		{
			detectionMade = EDetectionType::RightForward;
		}
	}
	else if (bBlockedLeft)
	{
		detectionMade = EDetectionType::Left;
	}
	else if (bBlockedRight)
	{
		detectionMade = EDetectionType::Right;
	}

	if (CurrentDetectionType != detectionMade)
	{
		OnDetectionChanged.Broadcast(detectionMade);
		CurrentDetectionType = detectionMade;
	}
}

bool UURTVehicleDetection::IsActorBlockable(AActor* actor)
{
	if (!actor)
	{
		return false;
	}
	
	if (actor->IsA(AURTSpawnablePawn::StaticClass()))
	{
		AURTSpawnablePawn* SpawnableActor = Cast<AURTSpawnablePawn>(actor);
		if (SpawnableActor->GetType() == ESpawnableType::Vehicle)
		{
			if (SpawnableActor->GetCurrentStreet() == OwnerVehicle->GetCurrentStreet() ||
				OwnerVehicle->GetCurrentStreet()->IsStreetAConnection(SpawnableActor->GetCurrentStreet()))
			{
				return true;
			}
		}
		// TODO: Só deve ser true se o peão está a passar uma passadeira
		// talvez isto não seja a melhor forma de checkar peões mas ficar assim para já
		else if(SpawnableActor->GetType() == ESpawnableType::Person)
		{
			return true;
		}
	}
	// TODO: Arranjar maneira de ter este 400 dinâmico
	else if (FVector::DistSquared(GetOwner()->GetActorLocation(), actor->GetActorLocation()) < 400)
	{
		return true;
	}
	return false;
}

