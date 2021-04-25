// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehicle.h"
#include "../Components/URTVehicleFollowPathComponent.h"
#include "VehicleWheel.h"
#include "Components/BoxComponent.h"
#include "../Components/URTVehicleDetection.h"
#include "URTSpawnablePawn.h"
#include "URTBaseVehicle.generated.h"

USTRUCT()
struct FVehicleInputs
{
	GENERATED_USTRUCT_BODY()

	float VISteeringInput;
	float VIThrottleInput;
	bool VIbHandbrake;

	void SetInputs(float ThrottleInput, float SteeringInput, bool bHandbrake)
	{
		VISteeringInput = SteeringInput;
		VIThrottleInput = ThrottleInput;
		VIbHandbrake = bHandbrake;
	}
};

/**
 *
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API AURTBaseVehicle : public AURTSpawnablePawn
{
	GENERATED_BODY()

private:

	/* Our Follow Path Component */
	UPROPERTY(Category = BaseVehicle, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		UURTVehicleFollowPathComponent* FollowPathComponent;

	/**  The main skeletal mesh associated with this Vehicle */
	UPROPERTY(Category = BaseVehicle, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

	/** vehicle simulation component */
	UPROPERTY(Category = BaseVehicle, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UWheeledVehicleMovementComponent* VehicleMovement;

	/* Detection component */
	UPROPERTY(Category = BaseVehicle, EditAnywhere, meta = (AllowPrivateAccess = "true"))
		class UURTVehicleDetection* DetectionComponent;

public:
	AURTBaseVehicle(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	void SetupDefaultWheels();

	void Tick(float DeltaSeconds) override;

	void ApplyInputs(float ThrottleInput, float SteeringInput, bool bHandbrake);
	FVehicleInputs GetVehicleInputs();
	/* Vehicle lateral speed */
	UFUNCTION(BlueprintPure, Category = AURTBaseVehicle)
		float GetVehicleLateralSpeed();

	UFUNCTION(BlueprintPure, Category = AURTBaseVehicle)
		EDetectionType GetVehicleCurrentDetection();

	void OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned) override;
	void OnActorCollected() override;

	/** Util to get the wheeled vehicle movement component */
	class UWheeledVehicleMovementComponent* GetVehicleMovementComponent() const;

	/** Returns Mesh subobject **/
	class USkeletalMeshComponent* GetMesh() const { return Mesh; }
	/** Returns VehicleMovement subobject **/
	class UWheeledVehicleMovementComponent* GetVehicleMovement() const { return VehicleMovement; }


	float GetCurrentDistanceOnTheRoad() override;

private:
	FVehicleInputs VehicleInputs;

protected:
	void BeginPlay() override;

};
