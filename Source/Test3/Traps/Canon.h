// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Canon.generated.h"

UCLASS()
class TEST3_API ACanon : public AActor
{
	GENERATED_BODY()
private:
	UPROPERTY(VisibleAnywhere)
		USceneComponent* Root;

	UPROPERTY(EditDefaultsOnly)
		USkeletalMeshComponent* GunMechineMesh;

	UPROPERTY(VisibleAnywhere)
		UStaticMeshComponent* TCanon;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* Target1;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* Target2;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* FollowTarget;

	UPROPERTY()
		FTimerHandle TimeHandler;

	UPROPERTY()
		int TimerCounter = 0;

	UFUNCTION()
		void UpdateLookAt();

	UFUNCTION()
		void ChangeCanonTarget();
public:	
	// Sets default values for this actor's properties
	ACanon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
