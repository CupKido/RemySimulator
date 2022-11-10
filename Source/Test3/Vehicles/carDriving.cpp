// Fill out your copyright notice in the Description page of Project Settings.


#include "Vehicles/carDriving.h"
#include "carDriving.h"

// Sets default values
AcarDriving::AcarDriving()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AcarDriving::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AcarDriving::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AcarDriving::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Throttle", this, &AcarDriving::Throttle);
	PlayerInputComponent->BindAxis("Steer", this, &AcarDriving::Steer);
	PlayerInputComponent->BindAxis("LookUp", this, &AcarDriving::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AcarDriving::LookRight);

	PlayerInputComponent->BindAction("ToggleCamera", IE_Pressed, this, &AcarDriving::EquipButtonPressed);

}

void AcarDriving::Throttle(float Value)
{
	this->currentSpeed = FMath::FInterpTo(this->currentSpeed, this->maxSpeed * Value, FApp::GetDeltaTime, 4.0);
}

void AcarDriving::Steer(float Value)
{
	this->currentSteeringAngle = FMath::FInterpTo(this->currentSteeringAngle, this->maxSteeringAngle * Value, FApp::GetDeltaTime, 4.0);
}

void AcarDriving::LookRight(float Value)
{
}

void AcarDriving::LookUp(float Value)
{
}

void AcarDriving::ToggleCamera()
{
}

