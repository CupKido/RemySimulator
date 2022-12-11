// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "ChaosVehicleMovementComponent.h"
#include "SportCar.generated.h"

UCLASS()
class TEST3_API ASportCar : public APawn
{
	GENERATED_BODY()
private:
	UPROPERTY()
		class USceneComponent* Root;

	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class USkeletalMeshComponent* Mesh;

	UPROPERTY(Category = Vehicle, VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		class UChaosVehicleMovementComponent* VehicleMovementComponent;

	UPROPERTY()
		USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY()
		class UBoxComponent* Box = nullptr;

	UPROPERTY()
		class USpringArmComponent* SpringArm1;

	UPROPERTY()
		class UCameraComponent* FrontCamera;

	UPROPERTY()
		 USpringArmComponent* SpringArm;

	UPROPERTY()
		 UCameraComponent* BackCamera;

public:
	// Sets default values for this pawn's properties
	ASportCar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
