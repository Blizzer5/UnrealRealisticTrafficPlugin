// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "URTStreet.h"
#include "../Actors/URTTrafficLight.h"
#include "URTVehicleStreet.generated.h"

/**
 *
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API AURTVehicleStreet : public AURTStreet
{
	GENERATED_BODY()

public:
	/* This Street max speed (km/h) */
	UPROPERTY(Category = "Street | Speed", EditAnywhere, meta = (AllowPrivateAccess = "true"))
		float StreetMaxSpeed = 50.f;

	/* This street left turn */
	UPROPERTY(Category = "Street | Turn Streets", EditAnywhere)
		TArray<AURTVehicleStreet*> LeftTurnStreets;
	/* This street forward street */
	UPROPERTY(Category = "Street | Turn Streets", EditAnywhere)
		TArray<AURTVehicleStreet*> ForwardStreets;
	/* This street right turn */
	UPROPERTY(Category = "Street | Turn Streets", EditAnywhere)
		TArray<AURTVehicleStreet*> RightTurnStreets;

public:
	/** Return the max speed a vehicle can hit */
	UFUNCTION(BlueprintPure, Category = VehicleStreet)
		float GetMaxSpeed() const;

	UFUNCTION(BlueprintCallable, Category = VehicleStreet)
		void GetAvailableStreets(TArray<AURTVehicleStreet*>& StreetsArray);
protected:
	void BeginPlay() override;

private:
	UFUNCTION(CallInEditor, meta = (EditCondition = "!bLeftTurnCreated"))
		void CreateLeftTurnStreet();
	UFUNCTION(CallInEditor, meta = (EditCondition = "!bForwardTurnCreated"))
		void CreateForwardStreet();
	UFUNCTION(CallInEditor, meta = (EditCondition = "!bRightTurnCreated"))
		void CreateRightStreet();
};
