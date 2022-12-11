// Fill out your copyright notice in the Description page of Project Settings.


#include "SportCar.h"

// Sets default values
ASportCar::ASportCar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASportCar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASportCar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASportCar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

