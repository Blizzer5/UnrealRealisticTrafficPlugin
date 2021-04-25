// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "URTBasePedestrianAIController.generated.h"

class AURTBaseAIPerson;


/**
 * 
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API AURTBasePedestrianAIController : public AAIController
{
	GENERATED_BODY()
	
	AURTBasePedestrianAIController();

private:
	UBlackboardComponent* BlackboardComponent;
	UBehaviorTreeComponent* BehaviorTreeComponent;
	AURTBaseAIPerson* OwnerAsPerson;
protected:
	void OnPossess(APawn* InPawn) override;

public:
	void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	void SetLocationToMoveTo(FVector newLocation);
	void SetRunningFrom(AActor* DangerActor);

	void Tick(float DeltaTime) override;

	void StartBehaviorTree();
	void StopBehaviorTree();
};
