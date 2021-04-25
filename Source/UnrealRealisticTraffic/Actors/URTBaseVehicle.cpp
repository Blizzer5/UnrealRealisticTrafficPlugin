// Fill out your copyright notice in the Description page of Project Settings.


#include "URTBaseVehicle.h"
#include "WheeledVehicleMovementComponent.h"
#include "../Components/URTVehicleFollowPathComponent.h"
#include "../Wheels/URTVehicleWheelFront.h"
#include "../Wheels/URTVehicleWheelRear.h"
#include "WheeledVehicleMovementComponent4W.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Character.h"
#include "../Components/URTVehicleDetection.h"

AURTBaseVehicle::AURTBaseVehicle(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh");
	Mesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	Mesh->BodyInstance.bSimulatePhysics = true;
	Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
	Mesh->BodyInstance.bUseCCD = true;
	Mesh->bBlendPhysics = true;
	Mesh->SetGenerateOverlapEvents(true);
	Mesh->SetCanEverAffectNavigation(false);
	RootComponent = Mesh;

	VehicleMovement = CreateDefaultSubobject<UWheeledVehicleMovementComponent, UWheeledVehicleMovementComponent4W>("VehicleMovementComponent");
	VehicleMovement->SetIsReplicated(true); // Enable replication by default
	VehicleMovement->UpdatedComponent = Mesh;

	FollowPathComponent = CreateDefaultSubobject<UURTVehicleFollowPathComponent>("Follow Path Component");

	DetectionComponent = CreateDefaultSubobject<UURTVehicleDetection>("Detection Component");

	SetupDefaultWheels();

	Type = ESpawnableType::Vehicle;
}

void AURTBaseVehicle::SetupDefaultWheels()
{
	for (int i = 0; i < GetVehicleMovementComponent()->WheelSetups.Num(); ++i)
	{
		FWheelSetup WheelSetup = GetVehicleMovementComponent()->WheelSetups[i];
		if (i <= 1)
		{
			WheelSetup.WheelClass = UURTVehicleWheelFront::StaticClass();
			if (i == 0)
			{
				WheelSetup.BoneName = "Wheel_Front_Left";
			}
			else if (i == 1)
			{
				WheelSetup.BoneName = "Wheel_Front_Right";
			}
		}
		else
		{
			WheelSetup.WheelClass = UURTVehicleWheelRear::StaticClass();
			if (i == 2)
			{
				WheelSetup.BoneName = "Wheel_Rear_Left";
			}
			else if (i == 3)
			{
				WheelSetup.BoneName = "Wheel_Rear_Right";
			}
		}
	}
}

void AURTBaseVehicle::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AURTBaseVehicle::ApplyInputs(float ThrottleInput, float SteeringInput, bool bHandbrake)
{
	GetVehicleMovementComponent()->SetThrottleInput(ThrottleInput);
	GetVehicleMovementComponent()->SetSteeringInput(SteeringInput);
	GetVehicleMovementComponent()->SetHandbrakeInput(bHandbrake);

	VehicleInputs.SetInputs(ThrottleInput, SteeringInput, bHandbrake);
}

FVehicleInputs AURTBaseVehicle::GetVehicleInputs()
{
	return VehicleInputs;
}

float AURTBaseVehicle::GetVehicleLateralSpeed()
{
	return GetActorQuat().UnrotateVector(GetVelocity()).Y;
}

EDetectionType AURTBaseVehicle::GetVehicleCurrentDetection()
{
	return DetectionComponent ? DetectionComponent->GetCurrentDetection() : EDetectionType::None;
}

void AURTBaseVehicle::OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned)
{
	Super::OnActorSpawnedInScene(StreetToAppear, currentDistance, LocationSpawned, RotationSpawned);

	SetCurrentStreet(StreetToAppear, currentDistance);

	FollowPathComponent->SetStreet(StreetToAppear);
	FollowPathComponent->SetComponentTickEnabled(true);

	GetMesh()->SetSimulatePhysics(true);

	// Just to make sure that the car physics state is as it should be
	GetVehicleMovementComponent()->RecreatePhysicsState();
}

void AURTBaseVehicle::OnActorCollected()
{
	Super::OnActorCollected();

	FollowPathComponent->SetStreet(nullptr);
	FollowPathComponent->SetComponentTickEnabled(false);

	GetMesh()->SetSimulatePhysics(false);
}

class UWheeledVehicleMovementComponent* AURTBaseVehicle::GetVehicleMovementComponent() const
{
	return VehicleMovement;
}

float AURTBaseVehicle::GetCurrentDistanceOnTheRoad()
{
	return FollowPathComponent->GetDistanceInRoad();
}

void AURTBaseVehicle::BeginPlay()
{
	Super::BeginPlay();

	FollowPathComponent->SetComponentTickEnabled(false);
}
