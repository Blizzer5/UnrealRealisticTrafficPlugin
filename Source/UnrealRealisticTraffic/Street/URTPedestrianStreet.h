// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "URTStreet.h"
#include "../Actors/URTTrafficLight.h"
#include "URTPedestrianStreet.generated.h"

/**
 *
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API AURTPedestrianStreet : public AURTStreet
{
	GENERATED_BODY()

	AURTPedestrianStreet();
public:
	/* This street left turn */
	UPROPERTY(Category = "Street | Turn Streets", EditAnywhere)
		TArray<AURTPedestrianStreet*> ForwardPathsToGoTo;

	UPROPERTY(Category = "Street | Turn Streets", EditAnywhere)
		TArray<AURTPedestrianStreet*> BehindPathsToGoTo;

	UPROPERTY(Category = "Street | Config", EditAnywhere)
	int MaxPedestrians = 5;


	AURTStreet* GetRandomForwardConnection();
	AURTStreet* GetRandomBehindConnection();

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
	UFUNCTION(BlueprintCallable, Category = VehicleStreet)
		void GetAvailableStreets(TArray<AURTPedestrianStreet*>& StreetsArray);
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
