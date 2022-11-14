// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Car.generated.h"


UCLASS()
class TEST3_API ACar : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	float MaxSpeed = 3000.0;
	float CurrentSpeed;
	float MaxSteeringAngle = 45.0;
	float CurrentSteeringAngle;
	float SpeedToRotate = 0.001;

	UPROPERTY(VisibleAnywhere, Category = "Car Body")
		class UAnimInstance* animation;

	UPROPERTY(VisibleAnywhere, Category = "Car Body")
		class USkeletalMeshComponent* body;

	UPROPERTY(VisibleAnywhere, Category = "Car TP")
		class USpringArmComponent* SpringArmComp;
	
	UPROPERTY(VisibleAnywhere, Category = "Car TP")
		class UCameraComponent* CameraTP;

	UPROPERTY(VisibleAnywhere, Category = "Car FP")
		class UCameraComponent* CameraFP;
};
