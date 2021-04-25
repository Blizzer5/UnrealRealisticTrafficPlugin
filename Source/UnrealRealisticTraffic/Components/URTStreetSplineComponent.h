// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "URTStreetSplineComponent.generated.h"

/**
 * 
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API UURTStreetSplineComponent : public USplineComponent
{
	GENERATED_BODY()
	
public:
    void AdaptLeftTurnForm();
    void AdaptForwardForm();
    void AdaptRightTurnForm();
};
