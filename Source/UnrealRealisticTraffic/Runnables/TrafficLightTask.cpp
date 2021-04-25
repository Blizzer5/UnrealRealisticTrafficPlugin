// Fill out your copyright notice in the Description page of Project Settings.


#include "TrafficLightTask.h"
#include "../Actors/URTTrafficLight.h"

TrafficLightTask::TrafficLightTask(AURTTrafficLight& TrafficLight, float startingSleepDelay)
    : TaskTrafficLight(TrafficLight),
    bIsRunning(false),
    sleepDelay(startingSleepDelay)
{
    const FString ThreadName = FString::Printf(TEXT("TrafficLightTask"));

    Thread = FRunnableThread::Create(this, *ThreadName);
}

uint32 TrafficLightTask::Run()
{
    while (bIsRunning)
    {
        FScopeLock Lock(&CriticalSection);
        if (bIsRunning && sleepDelay != 0) // make sure we were not told to exit during the wait
        {
            FPlatformProcess::Sleep(sleepDelay);
            sleepDelay = 0;
            AsyncTask(ENamedThreads::GameThread, [this]()
            {
                if (bIsRunning)
                {
                    TaskTrafficLight.ChangeLightState();
                }
            });
        }
    }

    return 0;
}

void TrafficLightTask::Stop()
{
    Exit();
}

void TrafficLightTask::Exit()
{
    if (!bIsRunning)
    {
        return;
    }

    bIsRunning = false;
}

bool TrafficLightTask::Init()
{
    bIsRunning = true;
    return true;
}

void TrafficLightTask::SetSleepAmount(float SleepAmount)
{
    sleepDelay = SleepAmount;
}

void TrafficLightTask::KillThread(bool bWaitToComplete)
{
    Thread->Kill(bWaitToComplete);
}
