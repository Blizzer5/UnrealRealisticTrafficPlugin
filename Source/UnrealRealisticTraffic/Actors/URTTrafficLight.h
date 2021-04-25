// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "../Runnables/TrafficLightTask.h"
#include "URTTrafficLight.generated.h"

class TrafficLightTask;

UENUM(BlueprintType)
enum class ELightState : uint8
{
	Green,
	Yellow,
	Red,
};
UENUM(BlueprintType)
enum class ELightChangeType : uint8
{
	// Show the yellow light before it goes green
	YellowToGreen,
	// Show the green light right after the red
	RedToGreen
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPedestrianLightTurnedGreen);


UCLASS()
class UNREALREALISTICTRAFFIC_API AURTTrafficLight : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AURTTrafficLight();
	~AURTTrafficLight();

	/** Time this traffic light will stay green */
	UPROPERTY(Category = "TrafficLight|Times", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float GreenTime = 20;
	/** Time this traffic light will stay yellow */
	UPROPERTY(Category = "TrafficLight|Times", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float YellowTime = 3;
	/** Time this traffic light will stay red */
	UPROPERTY(Category = "TrafficLight|Times", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float RedTime = 20;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	ELightState GetCurrentLightStateForVehicles();
	ELightState GetCurrentLightStateForPedestrians();
	
	void ChangeLightState();
	UFUNCTION(BlueprintImplementableEvent)
	void BPLightChanged();

	UFUNCTION(BlueprintImplementableEvent)
	void OnPreviousLightStateChanged();

	UPROPERTY(BlueprintAssignable)
	FOnPedestrianLightTurnedGreen OnPedestrianLightTurnedGreen;

	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	// This light state is for the vehicles. The pedestrian one is just the reverse of this
	// p.e if CurrentLightState == ELightState::Green then the pedestrian light is ELightState::Red
	UPROPERTY(ReplicatedUsing = "BPLightChanged", Category = "TrafficLight|Light", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ELightState CurrentLightState = ELightState::Green;
    UPROPERTY(Category = "TrafficLight|Light", EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
    ELightChangeType LightChangeType;

	UPROPERTY(Replicated, Category = "TrafficLight|Light", BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	ELightState PreviousLightState;
	TrafficLightTask* Task;
};
