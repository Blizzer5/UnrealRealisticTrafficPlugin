// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "HAL/Runnable.h"
#include "../Actors/URTTrafficLight.h"

class AURTTrafficLight;

/**
 * 
 */
class UNREALREALISTICTRAFFIC_API TrafficLightTask : public FRunnable
{
	
public:
	TrafficLightTask(AURTTrafficLight& TrafficLight, float startingSleepDelay);

	uint32 Run() override;
	void Stop() override;
	void Exit() override;
	bool Init() override;

	void SetSleepAmount(float SleepAmount);
	void KillThread(bool bWaitToComplete);
private:
	AURTTrafficLight& TaskTrafficLight;
	mutable FCriticalSection CriticalSection;
	FThreadSafeBool bIsRunning;
	FRunnableThread* Thread;

	float sleepDelay;
};
