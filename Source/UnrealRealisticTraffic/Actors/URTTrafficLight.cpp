// Fill out your copyright notice in the Description page of Project Settings.


#include "URTTrafficLight.h"
#include "../Runnables/TrafficLightTask.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AURTTrafficLight::AURTTrafficLight()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AURTTrafficLight::~AURTTrafficLight()
{
	if (Task != nullptr)
	{
		Task->KillThread(false);
		Task = nullptr;
	}
}

// Called when the game starts or when spawned
void AURTTrafficLight::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (CurrentLightState == ELightState::Yellow)
		{
			CurrentLightState = ELightState::Red;
			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red,
				FString::Printf(TEXT("%s started with a yellow light\nCurrently this is not supported.\nThis traffic light started as red instead"), *GetName()));
		}

		Task = new TrafficLightTask(*this, CurrentLightState == ELightState::Green ? GreenTime : RedTime);
	}
}

ELightState AURTTrafficLight::GetCurrentLightStateForVehicles()
{
	return CurrentLightState;
}

ELightState AURTTrafficLight::GetCurrentLightStateForPedestrians()
{
	return CurrentLightState != ELightState::Yellow ? CurrentLightState == ELightState::Green ? ELightState::Red : ELightState::Green : CurrentLightState;
}

void AURTTrafficLight::ChangeLightState()
{
	if (HasAuthority())
	{
		if (!Task)
		{
			return;
		}

		float newTimeToChange = 0.0;

		switch (CurrentLightState)
		{
		case ELightState::Green:
		{
			PreviousLightState = ELightState::Green;
			CurrentLightState = ELightState::Yellow;
			newTimeToChange = YellowTime;
		}
		break;
		case ELightState::Yellow:
		{
			if (PreviousLightState == ELightState::Green)
			{
				CurrentLightState = ELightState::Red;
				newTimeToChange = RedTime;

				OnPedestrianLightTurnedGreen.Broadcast();
			}
			else if (PreviousLightState == ELightState::Red && LightChangeType == ELightChangeType::YellowToGreen)
			{
				CurrentLightState = ELightState::Green;
				newTimeToChange = GreenTime;
			}
			PreviousLightState = ELightState::Yellow;
		}
		break;
		case ELightState::Red:
		{
			if (LightChangeType == ELightChangeType::YellowToGreen)
			{
				CurrentLightState = ELightState::Yellow;
				newTimeToChange = YellowTime;
			}
			else
			{
				CurrentLightState = ELightState::Green;
				newTimeToChange = GreenTime;
			}

			PreviousLightState = ELightState::Red;
		}
		break;
		}

		BPLightChanged();
		Task->SetSleepAmount(newTimeToChange);
	}
}

void AURTTrafficLight::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AURTTrafficLight, CurrentLightState);
	DOREPLIFETIME(AURTTrafficLight, PreviousLightState);
}

