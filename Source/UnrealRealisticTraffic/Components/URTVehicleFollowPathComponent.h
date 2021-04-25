// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "../Street/URTStreet.h"
#include "Delegates/DelegateCombinations.h"
#include "../Street/URTVehicleStreet.h"
#include "URTVehicleFollowPathComponent.generated.h"

USTRUCT(BlueprintType)
struct FURTPIDController
{
	GENERATED_USTRUCT_BODY()

		/** Proportional */
		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PID)
		float Kp;

	/** Integral */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PID)
		float Ki;

	/** Derivative */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = PID)
		float Kd;

	/** The last computed error */
	float LastError;

	/** The sum of errors */
	float SumError;

	/** Constructor */
	FURTPIDController()
		: Kp(1.0f)
		, Ki(0.0f)
		, Kd(0.0f)
		, LastError(0.0f)
		, SumError(0.0f)
	{
	}

	/** Constructor with params */
	FURTPIDController(float InKp, float InKi, float InKd)
		: Kp(InKp)
		, Ki(InKi)
		, Kd(InKd)
		, LastError(0.0f)
		, SumError(0.0f)
	{
	}

	/** Simulates the PID controller */
	float SimulatePID(float InDeltaTime, float InWantedValue, float CurrentValue, bool bIsAngle = false)
	{
		const float Error = bIsAngle ? FMath::FindDeltaAngleDegrees(CurrentValue, InWantedValue) : InWantedValue - CurrentValue;
		SumError += Error * InDeltaTime;

		const float Derivative = (Error - LastError) / InDeltaTime;
		const float Output = Kp * Error + Ki * SumError + Kd * Derivative;

		LastError = Error;

		return Output;
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FURTDistanceInRoadGenerated, AURTStreet*, street, float, InDistanceInRoad);

/**
 * Path follow a sync task for checking distance in road
 */
class FURTComputeDistanceInRoadTask : public FNonAbandonableTask
{
	friend class FAsyncTask<FURTComputeDistanceInRoadTask>;

public:

	FURTComputeDistanceInRoadTask(FVector InLocation, class AURTStreet* InPath, FURTDistanceInRoadGenerated InDelegate)
		: Location(InLocation)
		, Path(InPath)
		, Delegate(InDelegate)
	{}

	/** Set data for reusable task */
	void SetData(FVector InLocation, class AURTStreet* InPath, FURTDistanceInRoadGenerated InDelegate);

protected:

	/** The location */
	FVector Location;

	/** The spline */
	class AURTStreet* Path;

	/** Delegate to trigger */
	FURTDistanceInRoadGenerated Delegate;

	/** Do the work */
	void DoWork();

	/** Stat id */
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FURTDistanceInRoadGenerated, STATGROUP_ThreadPoolAsyncTasks);
	}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNREALREALISTICTRAFFIC_API UURTVehicleFollowPathComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UURTVehicleFollowPathComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Do agent pre simulation update */
	void PrepareAgentStep(FVector2D& OutPosition, FVector2D& OutPrefVelocity, FVector2D& OutVelocity, float& OutMaxSpeed);

	/** Do agent post simulation update */
	void ReceiveAgentStep(FVector2D InNewDirection, float InNewSpeed);

	/** Clears a sync tasks */
	virtual void ClearAsyncTasks();

	/** Set whether we are in reverse */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		void SetReverse(bool bInReverse);

	/** Set the time we are in reverse when getting un stuck */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		void SetReverseTime(float InTime);

	/** Set the max speed reversing */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		void SetReverseMaxSpeed(float InMaxSpeed);

	/** True if currently reversing */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		bool IsReversing() const;

	/** Set whether vehicle should stop */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		virtual void Stop(bool bInStop);

	/** Get distance in road we currently are */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		virtual float GetDistanceInRoad() const;

	/** Set distance in spline */
	UFUNCTION(BlueprintCallable, Category = "URTFollowPathComponent")
		virtual void SetDistanceInRoad(float InDistance);

	/** The distance of the target point in the path */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "URTFollowPathComponent")
		float DistanceAhead = 400.f;
	/* Turn sample distance */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float TurnSampleDistance = 5000.f;
	/* Turn sample size */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float TurnSampleSize = 500.f;
	/* The max lateral offset */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float MaxLateralOffset = 300.f;
	/* The max lateral speed (cm/s) */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float MaxLateralSpeed = 1000.f;
	/* The max lateral speed (cm/s) */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float GoalRadius = 100.f;
	/* Time it takes to end reverse */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float ReverseTime = 1.f;
	/* Reverse Max Speed (km/h) */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		float ReverseMaxSpeed = 20.f;
	/* Current path we're in */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | Setup")
		AURTVehicleStreet* CurrentStreet;

	/** Delegate path follow distance in road being generated */
	UPROPERTY(BlueprintAssignable)
		FURTDistanceInRoadGenerated DistanceInRoadGenerated;

	void SetStreet(AURTStreet* InPath, float InInitialDistance = 0.0f, bool bInUpdate = true);
	AURTVehicleStreet* GetStreet() const;

	void ForceVehicleStop();
	void RemoveVehicleStopForced();
private:

	/** The current turn cost */
	float TurnCost;

	/** The current slope cost */
	float SlopeCost;
	float SlopeCostInternal;

	/** The current lateral speed cost */
	float LateralSpeedCost;

	/** Current distance in road */
	float DistanceInRoad;

	/** The current distance to road */
	float DistanceToRoad;

	/** The current target speed */
	float TargetSpeed;

	/** The current target speed */
	FVector2D PrefVelocity;

	/** The current road distance coefficient */
	float RoadDistanceCoef;

	/** True when reversing / moving backwards */
	bool bReversing;

	/** The time at which we started to reverse */
	float ReverseStartTime;

	/** The time at which we stop reverse */
	float ReverseStopTime;

	/** Stuck from moving forward time */
	float StuckTime;

	/** Location of team at last frame */
	FVector LastLoc;

	/** If we want to force this vehicle to stop (Has a car or person in front p.e) */
	bool bForceStop;



	/** The steering PID controller */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | PID")
		FURTPIDController SteeringPID = FURTPIDController(.7f, 0.01f, 0.5f);

	/** The throttle PID controller */
	UPROPERTY(EditAnywhere, Category = "URTFollowPathComponent | PID")
		FURTPIDController ThrottlePID = FURTPIDController(.7f, 0.01f, 0.5f);

#if !UE_BUILD_SHIPPING
	/** Debug graphs */
	FDebugFloatHistory SteeringHistory;
	FDebugFloatHistory ThrottleHistory;
#endif

protected:

	/** A sync task for computing distance in road */
	FAsyncTask<FURTComputeDistanceInRoadTask>* DistanceInRoadTask;

	/** Trace handle and delegate */
	FTraceDelegate SlopeTraceDelegate;
	FTraceHandle SlopeTraceHandle;

	/**
	* Reversing stuck state
	*/
	virtual void UpdateStuckState(float InDeltaMove, float InDeltaTime);

	/**
	* Reversing state handling
	*/
	virtual void UpdateReverseState(float InDeltaTime);


	/**
		* Apply vehicle inputs given the deltas
	*/
	virtual void ApplyVehicleInput(float InCurYaw, float InTargetYaw, float InCurSpeed, float InTargetSpeed, bool bMovingBackwards, bool bStop, float InDeltaTime);

	/**
	* Get whether if vehicle should stop for any reason
	* @return True if vehicle should stop, false if not
	*/
	virtual bool ShouldStop() const;

	/**
		* Get the vehicle max speed
		* @return The vehicle max speed that cannot be overshot
	*/
	virtual float GetMaxSpeed() const;

	/**
	* Get the vehicle max speed when reversing
	* @return The vehicle max speed that cannot be overshot
	*/
	virtual float GetMaxReverseSpeed() const;

	/**
	* Get the distance ahead to use, read from config and apply offset depending on current velocity
	* @return The distance we are targeting
	*/
	virtual float GetDistanceAhead(float InSpeed) const;

	/** Get the distance and size of the turn samples */
	virtual void GetTurnSamplesDistanceAndSize(float InSpeed, float& OutDistance, float& OutSize) const;

	/**
	* Get the collision channel to use for collision avoidance
	* @return The collision channel
	*/
	virtual ECollisionChannel GetCollisionChannel() const;

	/** True if stuck and cannot move forward */
	virtual bool IsStuck() const;

	/**
	* Stop state changed
	* @param True if should stop, false if not
	*/
	virtual void OnStopStateChanged(bool bInStop);

	/** Slope trace */
	void DoSlopeTrace(const FVector InStart, const FVector InEnd);

	/** Slope cost */
	float ComputeSlopeCost(const FHitResult& InHit) const;

	/** Slope trace is done */
	virtual void OnSlopeTrace(const FTraceHandle& Handle, FTraceDatum& Data);

	/** Reset state */
	virtual void ResetState();

	/** The path end is reached */
	virtual void OnPathEnded();

#if WITH_EDITORONLY_DATA
	/** Draw the debug visualizers */
	virtual void DrawDebugVisualizers();
#endif // WITH_EDITOR

private:

	/** Distance in road is updated */
	UFUNCTION()
		void OnDistanceInRoadUpdated(AURTStreet* Street, float InDistanceInRoad);

	/** Do a sync task and get distance in road */
	void AsyncComputeDistanceInRoad();

	/** Compute the turn cost */
	float ComputeTurnCost(FVector InCurrentDirection, const TArray<FVector>& InSamples, bool bInAverage = false) const;

	/** Get direction samples from start to end distance in road, with precision */
	void GetRoadDirectionSamples(TArray<FVector>& OutSamples, float InStartDistance, float InEndDistance, float InSampleSize);
	void GetRoadDirectionSamplesReverse(TArray<FVector>& OutSamples, float InStartDistance, float InEndDistance, float InSampleSize);

	class AURTBaseVehicle* GetVehicle() const;
public:
	void InitializeComponent() override;


	void UninitializeComponent() override;

};
