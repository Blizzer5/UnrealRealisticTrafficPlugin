// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AITypes.h"
#include "Navigation/PathFollowingComponent.h"
#include "../Controllers/URTBasePedestrianAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "URTPersonBrain.generated.h"

class AURTBaseAIPerson;

UENUM(BlueprintType)
enum class EPedestrianMovementState : uint8 {
	Stopped,
	Walking,
	RunningFrom,
	Seated,
	Swimming,
	Driving,
};

UENUM(BlueprintType)
enum class EPedestrianObjective : uint8 {
	None,
	Walk,
	RunFrom,
	ToSeat,
	ToSwim,
	ToDrive,
};

UENUM(BlueprintType)
enum class EStreetUsage : uint8 {
	Forward,
	Reverse,
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNREALREALISTICTRAFFIC_API UURTPersonBrain : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UURTPersonBrain();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void UpdateMoveVariables(FVector moveToLocation, float speed);
	UFUNCTION()
	void OnMoveToCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result);

	UFUNCTION()
	void OnPedestrianLightTurnedGreen();

	void SetNextStreetToMoveTo();


	UFUNCTION(BlueprintCallable, Category = "State")
	EPedestrianMovementState GetPedestrianState();
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPedestrianState(EPedestrianMovementState newState); 

	UFUNCTION(BlueprintCallable, Category = "State")
	EPedestrianObjective GetPedestrianCurrentObjective();
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetPedestrianObjective(EPedestrianObjective newObjective);

	UFUNCTION(BlueprintCallable, Category = "State")
	EStreetUsage GetStreetUsage();
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetStreetUsage(EStreetUsage newStreetUsage);

	void Reset();

private:
	void OnGotScared(AActor* ScaredBy);

private:
	AURTBaseAIPerson* OwnerAsPerson;
	AURTBasePedestrianAIController* OwnerController;
	UCharacterMovementComponent* OwnerMovementComponent;

	EPedestrianMovementState CurrentPedestrianState = EPedestrianMovementState::Stopped;
	EStreetUsage StreetUsage = EStreetUsage::Forward;
	UPROPERTY(EditAnywhere, Category = "State")
	EPedestrianObjective PedestrianObjective = EPedestrianObjective::None;
public:
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

};
