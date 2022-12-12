// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Actor.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMesh.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "PlanePilot.generated.h"

UCLASS()
class TEST3_API APlanePilot : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APlanePilot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	// ** plane mesh start **
	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* Fuselage;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* Glass;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* AileronL;
	
	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* AileronR;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* RudderL;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* RudderR;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* ElevatorL;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* ElevatorR;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* FlapsL;

	UPROPERTY(EditAnywhere, Category = "Jet Body")
		UStaticMeshComponent* FlapsR;
	// ** plane mesh end **


	// plane spring arm
	UPROPERTY(VisibleAnywhere, Category = "Jet Body")
		class USpringArmComponent* SpringArmComp;

	// plane camera
	UPROPERTY(VisibleAnywhere, Category = "Jet Body")
		class UCameraComponent* CameraComp;

	// plane collision box to enter if collided
	//UPROPERTY(VisibleAnywhere, Category = "Jet Body")
	//	class UCollisionBox* Box;
	
	UPROPERTY(EditAnywhere, Category = "Jet Body")
		class UTextRenderComponent* TextToEnter;

	// constants start
	UPROPERTY(EditAnywhere)
	float maxThrustSpeed = 10000.0;

	UPROPERTY(EditAnywhere)
	float minThrustSpeed = 4000.0;

	UPROPERTY(EditAnywhere)
	float thrustMultiplier = 2500.0;
	const float gravity= 981.0;
	const float drag= 0.25;

	// control surfaces constants
	const float maxFlapPitch = 10.0;
	const float maxElevatorPitch = 25.0;
	const float maxRudderYaw = 45.0;
	const float maxAileronPitch = 45.0;
	//constants end

	float thrustSpeed = 0.0;
	float currentSpeed = 0.0;
	float appliedGravity = 0.0;
	float targetYaw = 0.0;
	float currentYaw = 0.0;
	float targetPitch = 0.0;
	float currentPitch = 0.0;
	float targetRoll = 0.0;
	float currentRoll = 0.0;

private:
	void UpdatePosition(float DeltaSeconds);
	void UpdateYaw(float Value, float DeltaSeconds);
	void UpdatePitch(float Value, float DeltaSeconds);
	void UpdateRoll(float Value, float DeltaSeconds);
	void PrintVariables();
	void Thrust(float Value);
	void Turn(float Value);
	void Pitch(float Value);
	void Roll(float Value);


	UPROPERTY(EditAnywhere, Category = "ThrusterParticles")
		UNiagaraSystem* ThrusterSystem;


	UPROPERTY(EditAnywhere, Category = "Jet Wheels")
		UStaticMeshComponent* wheels;

};
