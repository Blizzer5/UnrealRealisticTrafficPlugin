// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavigationInvokerComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "URTSpawnableCharacter.h"
#include "../Components/URTPersonBrain.h"
#include "URTBaseAIPerson.generated.h"

class AURTBasePedestrianAIController;

/**
 * 
 */
UCLASS()
class UNREALREALISTICTRAFFIC_API AURTBaseAIPerson : public AURTSpawnableCharacter
{
	GENERATED_BODY()
	
	AURTBaseAIPerson();

	/**  Component to know what to do with this person */
	UPROPERTY(Category = BasePerson, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UURTPersonBrain* PersonBrain;

	/**  Component to generate navmesh around this person */
	UPROPERTY(Category = BasePerson, EditAnywhere, meta = (AllowPrivateAccess = "true"))
	UNavigationInvokerComponent* NavigationInvoker;


public:
	UURTPersonBrain* GetPersonBrain();

	void OnActorSpawnedInScene(AURTStreet* StreetToAppear, float currentDistance, FVector LocationSpawned, FRotator RotationSpawned) override;
	void OnActorCollected() override;
	void SetCurrentStreet(AURTStreet* newStreet, float distance) override;

	float GetDistanceToMoveTo();

	UPROPERTY(EditAnywhere, Category = Behavior)
	class UBehaviorTree* PersonBehaviorTree;


	void Tick(float DeltaTime) override;

private:
	AURTBasePedestrianAIController* PersonController;

protected:
	void BeginPlay() override;

};
