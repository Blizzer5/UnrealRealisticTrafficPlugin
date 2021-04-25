// Fill out your copyright notice in the Description page of Project Settings.


#include "URTVehicleFollowPathComponent.h"
#include "../Actors/URTBaseVehicle.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"
#include "../Street/URTStreet.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "URTVehicleDetection.h"

#define FOLLOWPATHDEBUG WITH_EDITOR && 0

// Sets default values for this component's properties
UURTVehicleFollowPathComponent::UURTVehicleFollowPathComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = ETickingGroup::TG_PrePhysics;

	bAutoActivate = true;
	bAutoRegister = true;

	bWantsInitializeComponent = true;
}


// Called when the game starts
void UURTVehicleFollowPathComponent::BeginPlay()
{
	Super::BeginPlay();

	ReverseMaxSpeed *= 27.77;

}


// Called every frame
void UURTVehicleFollowPathComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (!CurrentStreet)
	{
		return;
	}

#if FOLLOWPATHDEBUG
	DrawDebugVisualizers();
#endif

	FVector2D Loc = FVector2D::ZeroVector;
	FVector2D PrefVel = FVector2D::ZeroVector;
	FVector2D Vel = FVector2D::ZeroVector;
	float MaxSpeed = 0.0f;

	PrepareAgentStep(Loc, PrefVel, Vel, MaxSpeed);
	FVector2D Normal = Vel.GetSafeNormal();
	ReceiveAgentStep(Normal, PrefVel.Size());
}

void UURTVehicleFollowPathComponent::InitializeComponent()
{
	Super::InitializeComponent();

	SlopeTraceDelegate.BindUObject(this, &UURTVehicleFollowPathComponent::OnSlopeTrace);
	DistanceInRoadGenerated.AddDynamic(this, &UURTVehicleFollowPathComponent::OnDistanceInRoadUpdated);
}

void UURTVehicleFollowPathComponent::UninitializeComponent()
{
	Super::UninitializeComponent();

	SlopeTraceDelegate.Unbind();

	DistanceInRoadGenerated.RemoveDynamic(this, &UURTVehicleFollowPathComponent::OnDistanceInRoadUpdated);

	ClearAsyncTasks();
}

void UURTVehicleFollowPathComponent::PrepareAgentStep(FVector2D& OutPosition, FVector2D& OutPrefVelocity, FVector2D& OutVelocity, float& OutMaxSpeed)
{
	AURTBaseVehicle* MyVehicle = GetVehicle();
	if (MyVehicle == nullptr || CurrentStreet == nullptr || CurrentStreet->StreetSpline == nullptr)
	{
		return;
	}

	if (!IsActive() || MyVehicle == nullptr)
	{
		return;
	}

	const float InDeltaTime = GetWorld()->GetDeltaSeconds();
	UpdateReverseState(InDeltaTime);

	const FVector VehicleZeroLoc = MyVehicle->GetActorLocation();
	const FVector VehicleVelocity = MyVehicle->GetVelocity();
	const float VehicleMovementDelta = (VehicleZeroLoc - LastLoc).Size();
	const bool bStop = ShouldStop();
	const bool bGoingReverse = IsReversing();
	const FVector VehicleLoc = MyVehicle->GetActorLocation();
	const float CurTime = UGameplayStatics::GetWorldDeltaSeconds(this);
	const float DotInRoad = FVector::DotProduct(CurrentStreet->StreetSpline->GetRotationAtDistanceAlongSpline(DistanceInRoad, ESplineCoordinateSpace::World).Vector(), VehicleVelocity.GetSafeNormal());

	// Keep current frame location
	LastLoc = VehicleZeroLoc;

	// Request distance in road update in a sync task
	AsyncComputeDistanceInRoad();

#if FOLLOWPATHDEBUG
	DrawDebugSphere(GetWorld(), CurrentStreet->StreetSpline->GetLocationAtDistanceAlongSpline(DistanceInRoad, ESplineCoordinateSpace::World), GoalRadius, 16, FColor::Green, false, 0.0f, 0, 5.0f);
#endif // WITH_EDITOR

	FVector DistToRoadLoc;
	FRotator DistToRoadRot;
	DistToRoadLoc = CurrentStreet->StreetSpline->GetLocationAtDistanceAlongSpline(DistanceInRoad, ESplineCoordinateSpace::World);
	DistToRoadRot = CurrentStreet->StreetSpline->GetRotationAtSplinePoint(DistanceInRoad, ESplineCoordinateSpace::World);
	DistanceToRoad = FVector::Dist2D(DistToRoadLoc, VehicleLoc);

	// Compute the target distance ahead
	const float ComputedDistanceAhead = GetDistanceAhead(VehicleVelocity.Size());
	const float TargetDistanceInRoad = CurrentStreet->ComputeDistanceWith(DistanceInRoad, bGoingReverse ? -ComputedDistanceAhead : ComputedDistanceAhead, false);

	// Compute the target location and rotation in road we want to be at
	FVector TargetLocationInRoad;
	FRotator TargetRotationInRoad;
	CurrentStreet->GetLocationAndRotationAt(TargetDistanceInRoad, TargetLocationInRoad, TargetRotationInRoad);

	// Apply lateral offset
	TargetLocationInRoad += FVector::CrossProduct(TargetRotationInRoad.Vector(), FVector::UpVector);

	const FVector WantedForward = (TargetLocationInRoad - VehicleLoc).GetSafeNormal() * (bGoingReverse ? -1.0f : 1.0f);
	const FRotator TargetRot = FRotationMatrix::MakeFromZX(FVector::UpVector, WantedForward).Rotator();
	const float RoadOffsetCoef = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(DistanceToRoad, MaxLateralOffset, MaxLateralOffset * 4), 0.0f, 1.0f);

	bool bIsSpeedLimited = bGoingReverse;
	const float MaxSpeed = bGoingReverse ? -GetMaxReverseSpeed() : GetMaxSpeed();

	if (!bStop)
	{
		UpdateStuckState(VehicleMovementDelta, InDeltaTime);
	}

	float CurrentTurnSampleDistance = 0.0f;
	float CurrentTurnSampleSize = 0.0f;
	GetTurnSamplesDistanceAndSize(VehicleVelocity.Size(), CurrentTurnSampleDistance, CurrentTurnSampleSize);

	TArray<FVector> DirectionSamples;

	if (!bGoingReverse)
	{
		GetRoadDirectionSamples(DirectionSamples, DistanceInRoad, CurrentStreet->ComputeDistanceWith(DistanceInRoad, CurrentTurnSampleDistance), CurrentTurnSampleSize);
	}
	else
	{
		GetRoadDirectionSamplesReverse(DirectionSamples, DistanceInRoad, CurrentStreet->ComputeDistanceWith(DistanceInRoad, -CurrentTurnSampleDistance), CurrentTurnSampleSize);
	}

	// Trace slope cost
	if (!GetWorld()->IsTraceHandleValid(SlopeTraceHandle, false))
	{
		DoSlopeTrace(TargetLocationInRoad + FVector(0.0f, 0.0f, 5000.0f), TargetLocationInRoad + FVector(0.0f, 0.0f, -5000.0f));
	}

	TurnCost = bIsSpeedLimited ? 1.0f : ComputeTurnCost(MyVehicle->GetActorForwardVector(), DirectionSamples);
	SlopeCost = bIsSpeedLimited ? 1.0f : FMath::FInterpConstantTo(SlopeCost, SlopeCostInternal, InDeltaTime, SlopeCostInternal < SlopeCost ? 1.0f : 0.1f);
	RoadDistanceCoef = bIsSpeedLimited ? 1.0f : FMath::Lerp(1.0f, 0.5f, RoadOffsetCoef);
	LateralSpeedCost = FMath::Clamp(UKismetMathLibrary::NormalizeToRange(MaxLateralSpeed, 0.0f, FMath::Abs(MyVehicle->GetVehicleLateralSpeed())), 0.5f, 1.0f);

	const float Cost = FMath::Clamp(TurnCost * SlopeCost * RoadDistanceCoef * LateralSpeedCost, 0.2f, 1.0f);

	TargetSpeed = bStop ? 0.0f : FMath::Clamp(MaxSpeed * Cost, bGoingReverse ? MaxSpeed : 0.0f, MaxSpeed);
	PrefVelocity = FVector2D(TargetRot.Vector() * TargetSpeed);

	OutPosition = FVector2D(GetVehicle()->GetActorLocation().X, GetVehicle()->GetActorLocation().Y);
	OutPrefVelocity = PrefVelocity;
	OutVelocity = FVector2D(GetVehicle()->GetVelocity());
	OutMaxSpeed = FMath::Abs(TargetSpeed);
}

void UURTVehicleFollowPathComponent::ReceiveAgentStep(FVector2D InNewDirection, float InNewSpeed)
{
	AURTBaseVehicle* MyVehicle = GetVehicle();
	if (MyVehicle == nullptr)
		return;

	if (!IsActive())
		return;

	//if (!CanUseAvoidance())
	//{
		// Check if wanted direction is inverted in relation to our preferred direction
	const bool bInvertedDirection = FVector2D::DotProduct(PrefVelocity.GetSafeNormal(), InNewDirection) <= 0.0f;

	// If going in the opposite direction set speed to zero, we have an obstacle in front and we cannot use avoidance
	InNewSpeed = bInvertedDirection || ShouldStop() ? 0.0f : InNewSpeed;

	// Set preferred velocity, before changing to the ORCA one
	InNewDirection = PrefVelocity.GetSafeNormal();
	// }

	const float InDeltaTime = GetWorld()->GetDeltaSeconds();
	const FVector OldVel = MyVehicle->GetVelocity().GetSafeNormal();
	const float DotForwardOld = FVector2D::DotProduct(FVector2D(MyVehicle->GetActorForwardVector()), FVector2D(OldVel));
	const float DotForwardNew = FVector2D::DotProduct(FVector2D(MyVehicle->GetActorForwardVector()), InNewDirection);
	const float OldSpeed = DotForwardOld > 0.0f ? MyVehicle->GetVelocity().Size() : MyVehicle->GetVelocity().Size() * -1.0f;
	const float NewSpeed = DotForwardNew > 0.0f ? InNewSpeed : InNewSpeed * -1.0f;
	const float OldYaw = bReversing ? (MyVehicle->GetActorRotation().Quaternion() * FRotator(0.0f, 180.0f, 0.0f).Quaternion()).Rotator().Yaw : MyVehicle->GetActorRotation().Yaw;
	const float NewYaw = FVector(InNewDirection.X, InNewDirection.Y, 0.0f).Rotation().Yaw;

	ApplyVehicleInput(OldYaw, NewYaw, OldSpeed, NewSpeed, DotForwardNew <= 0.0f, ShouldStop(), InDeltaTime);

	if (!ShouldStop() && (DistanceInRoad >= CurrentStreet->GetLength() - GoalRadius || FVector::Dist2D(MyVehicle->GetActorLocation(), CurrentStreet->GetEndLocation()) < GoalRadius))
	{
		OnPathEnded();
	}
}

void UURTVehicleFollowPathComponent::ClearAsyncTasks()
{
	if (DistanceInRoadTask != nullptr)
	{
		DistanceInRoadTask->EnsureCompletion();
		delete DistanceInRoadTask;
		DistanceInRoadTask = nullptr;
	}
}

class AURTVehicleStreet* UURTVehicleFollowPathComponent::GetStreet() const
{
	return CurrentStreet;
}

void UURTVehicleFollowPathComponent::ForceVehicleStop()
{
	bForceStop = true;
}

void UURTVehicleFollowPathComponent::RemoveVehicleStopForced()
{
	bForceStop = false;
}

void UURTVehicleFollowPathComponent::SetStreet(class AURTStreet* InPath, float InInitialDistance /*= 0.0f*/, bool bInUpdate /*= true*/)
{
	AURTVehicleStreet* VehicleStreet = Cast<AURTVehicleStreet>(InPath);
	if (CurrentStreet != VehicleStreet)
	{
		if (CurrentStreet)
		{
			CurrentStreet->RemoveActorOnThisStreet(GetVehicle());
		}

		CurrentStreet = VehicleStreet;
		DistanceInRoad = InInitialDistance;
		DistanceToRoad = 0.0f;
		LastLoc = InPath != nullptr ? CurrentStreet->StreetSpline->GetLocationAtDistanceAlongSpline(InInitialDistance, ESplineCoordinateSpace::World) : FVector::ZeroVector;

		if (CurrentStreet)
		{
			CurrentStreet->AddActorOnThisStreet(GetVehicle());
		}

		GetVehicle()->SetCurrentStreet(CurrentStreet, 0.f);
	}
}

void UURTVehicleFollowPathComponent::SetReverse(bool bInReverse)
{
	if (bReversing != bInReverse)
	{
		bReversing = bInReverse;

		if (bReversing)
		{
			ReverseStartTime = GetWorld()->TimeSeconds;
			ReverseStopTime = 0.0f;
		}
		else
		{
			ReverseStartTime = 0.0f;
			ReverseStopTime = GetWorld()->TimeSeconds;
		}
	}
}

void UURTVehicleFollowPathComponent::SetReverseTime(float InTime)
{
	ReverseTime = InTime;
}

void UURTVehicleFollowPathComponent::SetReverseMaxSpeed(float InMaxSpeed)
{
	ReverseMaxSpeed = InMaxSpeed;
}

bool UURTVehicleFollowPathComponent::IsReversing() const
{
	return bReversing;
}

void UURTVehicleFollowPathComponent::Stop(bool bInStop)
{
	OnStopStateChanged(bInStop);
}

float UURTVehicleFollowPathComponent::GetDistanceInRoad() const
{
	return DistanceInRoad;
}

void UURTVehicleFollowPathComponent::SetDistanceInRoad(float InDistance)
{
	DistanceInRoad = InDistance;
}

void UURTVehicleFollowPathComponent::OnDistanceInRoadUpdated(AURTStreet* Street, float InDistanceInRoad)
{
	if (CurrentStreet == Street)
	{
		DistanceInRoad = IsReversing() ? FMath::Min(DistanceInRoad, InDistanceInRoad) : FMath::Max(DistanceInRoad, InDistanceInRoad);
	}
}

void UURTVehicleFollowPathComponent::AsyncComputeDistanceInRoad()
{
	// Task was not yet started
	if (DistanceInRoadTask == nullptr)
	{
		DistanceInRoadTask = new FAsyncTask<FURTComputeDistanceInRoadTask>(GetVehicle()->GetActorLocation(), CurrentStreet, DistanceInRoadGenerated);
		check(DistanceInRoadTask != nullptr);

		if (DistanceInRoadTask != nullptr)
		{
			DistanceInRoadTask->StartBackgroundTask();
		}
	}
	// Task was started and it is done doing work, re use it and request more work
	else if (DistanceInRoadTask->IsDone())
	{
		DistanceInRoadTask->GetTask().SetData(GetVehicle()->GetActorLocation(), CurrentStreet, DistanceInRoadGenerated);
		DistanceInRoadTask->StartBackgroundTask();
	}
}

float UURTVehicleFollowPathComponent::ComputeTurnCost(FVector InCurrentDirection, const TArray<FVector>& InSamples, bool bInAverage /*= false*/) const
{
	if (InSamples.Num() == 0)
	{
		return 1.0f;
	}

	float Cost = 1.0f;

	if (bInAverage)
	{
		// Take average from samples
		float Sum = 0.0f;

		for (FVector Dir : InSamples)
		{
			Sum += FVector::DotProduct(InCurrentDirection, Dir);
		}

		const float AverageDot = Sum / InSamples.Num();
		Cost = UKismetMathLibrary::NormalizeToRange(AverageDot, -1.0f, 1.0f);
	}
	else
	{
		// Take highest from samples
		for (FVector Dir : InSamples)
		{
			float PossibleMax = FVector::DotProduct(InCurrentDirection, Dir);

			if (PossibleMax < Cost)
			{
				Cost = UKismetMathLibrary::NormalizeToRange(PossibleMax, -1.0f, 1.0f);
			}
		}
	}

	return FMath::Clamp(Cost, 0.25f, 1.0f);
}

void UURTVehicleFollowPathComponent::GetRoadDirectionSamples(TArray<FVector>& OutSamples, float InStartDistance, float InEndDistance, float InSampleSize)
{
	float CurDist = InStartDistance;

	if (CurDist > InEndDistance)
		InEndDistance = CurrentStreet->StreetSpline->GetSplineLength() + InEndDistance;

	while (CurDist < InEndDistance)
	{
		if (CurDist > InEndDistance)
			CurDist = InEndDistance;

		FVector Loc = CurrentStreet->StreetSpline->GetLocationAtDistanceAlongSpline(CurDist, ESplineCoordinateSpace::World);
		FRotator Rot = CurrentStreet->StreetSpline->GetRotationAtDistanceAlongSpline(CurDist, ESplineCoordinateSpace::World);

		OutSamples.Add(Rot.Vector());

		CurDist += InSampleSize;

#if FOLLOWPATHDEBUG
		if (true)
		{
			DrawDebugSphere(GetWorld(), Loc, 50.0f, 16, FColor::Yellow, false, 0.0f, 0, 3.0f);
		}
#endif // WITH_EDITOR
	}
}

void UURTVehicleFollowPathComponent::GetRoadDirectionSamplesReverse(TArray<FVector>& OutSamples, float InStartDistance, float InEndDistance, float InSampleSize)
{
	float CurDist = InStartDistance;

	if (CurDist < InEndDistance)
		InEndDistance = CurrentStreet->StreetSpline->GetSplineLength() - InEndDistance;

	while (CurDist > InEndDistance)
	{
		if (CurDist < InEndDistance)
			CurDist = InEndDistance;

		FVector Loc = CurrentStreet->StreetSpline->GetLocationAtDistanceAlongSpline(CurDist, ESplineCoordinateSpace::World);
		FRotator Rot = CurrentStreet->StreetSpline->GetRotationAtDistanceAlongSpline(CurDist, ESplineCoordinateSpace::World);

		OutSamples.Add(Rot.Vector());

		CurDist -= InSampleSize;

#if FOLLOWPATHDEBUG
		if (true)
		{
			DrawDebugSphere(GetWorld(), Loc, 50.0f, 16, FColor::Yellow, false, 0.0f, 0, 3.0f);
		}
#endif // WITH_EDITOR
	}
}

AURTBaseVehicle* UURTVehicleFollowPathComponent::GetVehicle() const
{
	return Cast<AURTBaseVehicle>(GetOwner());
}

void UURTVehicleFollowPathComponent::UpdateReverseState(float InDeltaTime)
{
	if (!IsReversing())
	{
		if (!ShouldStop() && IsStuck())
		{
			SetReverse(true);
		}
	}
	else
	{
		const float CurTime = GetWorld()->TimeSeconds;

		if (CurTime >= ReverseStartTime + ReverseTime)
		{
			SetReverse(false);
		}
	}
}

void UURTVehicleFollowPathComponent::UpdateStuckState(float InDeltaMove, float InDeltaTime)
{
	if (IsReversing())
	{
		StuckTime = 0.0f;
		return;
	}

	if (FMath::Abs(InDeltaMove) < 2.2f)
	{
		StuckTime += InDeltaTime;
	}
	else
	{
		StuckTime = 0.0f;
	}

	StuckTime = FMath::Clamp(StuckTime, 0.0f, MAX_FLT);
}

void UURTVehicleFollowPathComponent::ApplyVehicleInput(float InCurYaw, float InTargetYaw, float InCurSpeed, float InTargetSpeed, bool bMovingBackwards, bool bStop, float InDeltaTime)
{
	AURTBaseVehicle* MyVehicle = GetVehicle();
	if (MyVehicle == nullptr)
		return;

	// Default inputs
	float SteeringInput = 0.0f;
	float ThrottleInput = 0.0f;
	bool bHandbrake = bStop;

	// If not told to stop, compute steering and throttle / brake
	if (!bStop)
	{
		// Steering
		if (InCurYaw != InTargetYaw)
		{
			SteeringInput = SteeringPID.SimulatePID(InDeltaTime, InTargetYaw, InCurYaw, true);
		}
	}

	// Throttle
	if (!FMath::IsNearlyEqual(InCurSpeed, InTargetSpeed, 0.01f))
	{
		ThrottleInput = ThrottlePID.SimulatePID(InDeltaTime, InTargetSpeed, InCurSpeed);
	}


	// We are moving backwards, this does not mean Reverse
	// Moving backwards means velocity is opposite to target direction
	if (bMovingBackwards)
	{
		// If not reversing, invert throttle
		if (!IsReversing())
		{
			ThrottleInput *= bStop ? 1.0f : -1.0f;
		}
		// If reversing, invert steering
		else
		{
			SteeringInput *= -1.0f;
		}
	}
	// We are in reverse mode, invert both throttle and steering inputs
	else if (IsReversing())
	{
		ThrottleInput *= -1.0f;
		SteeringInput *= -1.0f;
	}

	// Clamp values
	SteeringInput = FMath::Clamp(SteeringInput, -1.f, 1.f);
	ThrottleInput = FMath::Clamp(ThrottleInput, -0.7f, 0.6f);

	// Apply inputs on vehicle
	MyVehicle->ApplyInputs(
		ThrottleInput,
		SteeringInput,
		bHandbrake ? 1.0f : 0.0f
	);
}

bool UURTVehicleFollowPathComponent::ShouldStop() const
{
	return bForceStop ||
		!CurrentStreet ||
		((CurrentStreet->GetLength() - DistanceInRoad) < UKismetMathLibrary::MapRangeClamped(GetVehicle()->GetVelocity().Size(), 0, 1000, 400, 1250) && CurrentStreet->GetCurrentStreetTrafficLightStateForVehicles() != ELightState::Green) ||
		GetVehicle()->GetVehicleCurrentDetection() != EDetectionType::None;
}

void UURTVehicleFollowPathComponent::OnStopStateChanged(bool bInStop)
{
	if (bInStop)
	{
		ApplyVehicleInput(0.0f, 0.0f, 0.0f, 0.0f, false, true, 0.0f);
	}
}

float UURTVehicleFollowPathComponent::GetMaxSpeed() const
{
	return (CurrentStreet->GetLength() - DistanceInRoad) > 1250 ? CurrentStreet->GetMaxSpeed() : CurrentStreet->GetMaxSpeed() * 0.5f;
}

float UURTVehicleFollowPathComponent::GetMaxReverseSpeed() const
{
	return ReverseMaxSpeed;
}

float UURTVehicleFollowPathComponent::GetDistanceAhead(float InSpeed) const
{
	InSpeed = FMath::Abs(InSpeed);

	if (IsReversing())
	{
		return DistanceAhead;
	}
	else if (InSpeed > 0.0f)
	{
		const float AddedDist = DistanceAhead;
		const float Alpha = UKismetMathLibrary::NormalizeToRange(InSpeed, 0.0f, IsReversing() ? GetMaxReverseSpeed() : GetMaxSpeed());

		return DistanceAhead + (AddedDist * Alpha);
	}
	else
	{
		return DistanceAhead;
	}
}

void UURTVehicleFollowPathComponent::GetTurnSamplesDistanceAndSize(float InSpeed, float& OutDistance, float& OutSize) const
{
	const float MinDist = TurnSampleDistance;
	const float MaxDist = MinDist * 2.0f;
	const float MinSize = TurnSampleSize;
	const float MaxSize = MinSize * 2.0f;
	const float Alpha = UKismetMathLibrary::NormalizeToRange(InSpeed, 0.0f, IsReversing() ? GetMaxReverseSpeed() : GetMaxSpeed());

	OutDistance = FMath::Lerp(MinDist, MaxDist, Alpha);
	OutSize = FMath::Lerp(MinSize, MaxSize, Alpha);
}

ECollisionChannel UURTVehicleFollowPathComponent::GetCollisionChannel() const
{
	return ECollisionChannel::ECC_Vehicle;
}

bool UURTVehicleFollowPathComponent::IsStuck() const
{
	return StuckTime > 4.0f;
}

void UURTVehicleFollowPathComponent::DoSlopeTrace(const FVector InStart, const FVector InEnd)
{
	FCollisionResponseParams ResponseParams;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());
	Params.bReturnPhysicalMaterial = false;

	SlopeTraceHandle = GetWorld()->AsyncLineTraceByChannel(EAsyncTraceType::Single, InStart, InEnd, GetCollisionChannel(), Params, ResponseParams, &SlopeTraceDelegate);
}

float UURTVehicleFollowPathComponent::ComputeSlopeCost(const FHitResult& InHit) const
{
	if (InHit.bBlockingHit)
	{
		AURTBaseVehicle* MyVehicle = GetVehicle();
		if (MyVehicle == nullptr || CurrentStreet == nullptr || CurrentStreet->StreetSpline == nullptr)
		{
			return 1.0f;
		}
		const FVector Up = GetVehicle() != nullptr ? GetVehicle()->GetActorUpVector() : FVector::UpVector;
		const float UpDot = FVector::DotProduct(InHit.ImpactNormal, Up);
		const float Cost = UKismetMathLibrary::NormalizeToRange(UpDot, 0.5f, 1.0f);

		return FMath::Clamp(Cost, 0.25f, 1.0f);
	}

	return 1.0f;
}

void UURTVehicleFollowPathComponent::OnSlopeTrace(const FTraceHandle& Handle, FTraceDatum& Data)
{
	if (Handle == SlopeTraceHandle)
	{
		if (Data.OutHits.IsValidIndex(0))
		{
			SlopeCostInternal = ComputeSlopeCost(Data.OutHits[0]);

#if FOLLOWPATHDEBUG
			if (true)
			{
				DrawDebugSphere(GetWorld(), Data.OutHits[0].ImpactPoint, 25.0f, 16, FColor::Red, false, 0.0f, 0, 3.0f);
				DrawDebugDirectionalArrow(GetWorld(), Data.OutHits[0].ImpactPoint, Data.OutHits[0].ImpactPoint + Data.OutHits[0].ImpactNormal * 250.0f, 250.0f, FColor::Red, false, 0.0f, 0, 10.0f);
	}
#endif // WITH_EDITOR
}
	}
}

void UURTVehicleFollowPathComponent::ResetState()
{
	SetReverse(false);
}

void UURTVehicleFollowPathComponent::OnPathEnded()
{
	TArray<AURTVehicleStreet*> StreetsArray;
	CurrentStreet->GetAvailableStreets(StreetsArray);
	if (StreetsArray.Num() > 0)
	{
		SetStreet(StreetsArray[FMath::RandRange(0, StreetsArray.Num() - 1)]);
	}
	else
	{
		SetStreet(nullptr);
		Stop(true);
	}
}

#if WITH_EDITORONLY_DATA
void UURTVehicleFollowPathComponent::DrawDebugVisualizers()
{
	#if FOLLOWPATHDEBUG
	AURTBaseVehicle* MyVehicle = GetVehicle();

	if (GEngine != nullptr && MyVehicle != nullptr && MyVehicle->IsLocallyControlled())
	{
		const float ThrottleInput = MyVehicle->GetVehicleInputs().VIThrottleInput;
		const float SteeringInput = MyVehicle->GetVehicleInputs().VISteeringInput;

		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Turn Cost : %f"), 1.0f - TurnCost));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Slope Cost : %f"), 1.0f - SlopeCost));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Road Distance Cost : %f"), 1.0f - RoadDistanceCoef));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Lateral Speed Cost : %f"), 1.0f - LateralSpeedCost));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Target Speed : %f"), TargetSpeed));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Current Speed : %f"), MyVehicle->GetVelocity().Size()));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Distance In Road : %f"), DistanceInRoad));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Distance To Road : %f"), DistanceToRoad));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Steering Input : %f"), SteeringInput));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Throttle Input : %f"), ThrottleInput));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Is Reversing : %s"), IsReversing() ? TEXT("YES") : TEXT("NO")));
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, *FString::Printf(TEXT("Should Stop : %s"), ShouldStop() ? TEXT("YES") : TEXT("NO")));
	}
	#endif
}
#endif // WITH_EDITORONLY_DATA

void FURTComputeDistanceInRoadTask::SetData(FVector InLocation, class AURTStreet* InPath, FURTDistanceInRoadGenerated InDelegate)
{
	Location = InLocation;
	Path = InPath;
	Delegate = InDelegate;
}

void FURTComputeDistanceInRoadTask::DoWork()
{
	if (Path != nullptr)
	{
		const float DistanceInRoad = Path->ComputeNearestDistanceToLocation(Location);

		if (Delegate.IsBound())
		{
			Delegate.Broadcast(Path, DistanceInRoad);
		}
	}
}
