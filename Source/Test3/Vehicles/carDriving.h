// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "carDriving.generated.h"

UCLASS()
class TEST3_API AcarDriving : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AcarDriving();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//Input funcs
	void Throttle(float Value);
	void Steer(float Value);
	void LookRight(float Value);
	void LookUp(float Value);
	void ToggleCamera();

protected:
	float maxSpeed = 3000.0;
	float currentSpeed = 0.0;
	float maxSteeringAngle = 45.0;
	float currentSteeringAngle = 0.0;
	float speedToRotate = 0.001;
};
