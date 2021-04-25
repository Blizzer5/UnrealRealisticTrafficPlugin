// Fill out your copyright notice in the Description page of Project Settings.


#include "URTBasePedestrianAIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "DrawDebugHelpers.h"
#include "../Actors/URTBaseAIPerson.h"

AURTBasePedestrianAIController::AURTBasePedestrianAIController()
{
	BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackBoardComp"));
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));

	bWantsPlayerState = true;
}

void AURTBasePedestrianAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	OwnerAsPerson = Cast<AURTBaseAIPerson>(InPawn);

	if(OwnerAsPerson->IsActive()) {
		StartBehaviorTree();
	}
}

void AURTBasePedestrianAIController::StartBehaviorTree()
{
	//start behavior
	if (OwnerAsPerson && OwnerAsPerson->PersonBehaviorTree) {
		BlackboardComponent->InitializeBlackboard(*OwnerAsPerson->PersonBehaviorTree->BlackboardAsset);

		BehaviorTreeComponent->StartTree(*OwnerAsPerson->PersonBehaviorTree);
	}
}

void AURTBasePedestrianAIController::StopBehaviorTree()
{
	//stop behavior
	if (OwnerAsPerson && OwnerAsPerson->PersonBehaviorTree) {
		BehaviorTreeComponent->StopTree(EBTStopMode::Safe);
	}
}

void AURTBasePedestrianAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);
}

void AURTBasePedestrianAIController::SetLocationToMoveTo(FVector newLocation)
{
	BlackboardComponent->SetValueAsVector("LocationToGoTo", newLocation);
}

void AURTBasePedestrianAIController::SetRunningFrom(AActor* DangerActor)
{
	BlackboardComponent->SetValueAsObject("RunFrom", DangerActor);
}

void AURTBasePedestrianAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//DrawDebugSphere(GetWorld(), BlackboardComponent->GetValueAsVector("LocationToGoTo"), 20.f, 16, FColor::Green, false, 0.0f, 0, 5.0f);
}
