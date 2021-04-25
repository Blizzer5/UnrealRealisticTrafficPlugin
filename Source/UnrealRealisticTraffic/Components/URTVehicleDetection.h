// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "URTVehicleDetection.generated.h"

class AURTBaseVehicle;

UENUM(BlueprintType)
enum class EDetectionType : uint8
{
	None,
	Left,
	Right,
	Forward,
	LeftForward,
	RightForward,
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDetectionChanged, EDetectionType, DetectionType);

UCLASS(Blueprintable, ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class UNREALREALISTICTRAFFIC_API UURTVehicleDetection : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UURTVehicleDetection();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Center")
		float DetectionCenterMultiplier = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Center")
		float DetectionCenterZOverride = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Sizes")
		float ForwardBackwardDetectionMultiplier = 2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Sizes")
		float DiagonalDetectionsMultiplier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Sizes")
		float DiagonalAngle = 20.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle Detection | Detection Sizes")
		TEnumAsByte<EDrawDebugTrace::Type> DebugTrace = EDrawDebugTrace::None;

	UFUNCTION(BlueprintCallable, Category = "Vehicle Detection | Functions")
		void SetIsReversing(bool bIsVehicleReversing);

	UFUNCTION(BlueprintCallable, Category = "Vehicle Detection | Functions")
		EDetectionType GetCurrentDetection();

	UPROPERTY(BlueprintAssignable, Category = "VehicleDetection")
		FOnDetectionChanged OnDetectionChanged;

private:
	void DoTrace();
	bool IsActorBlockable(AActor* actor);

	AURTBaseVehicle* OwnerVehicle;
	bool bIsReversing;
	EDetectionType CurrentDetectionType;
};
