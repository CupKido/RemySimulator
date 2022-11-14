// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"

// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->TargetArmLength = 800.f;

	body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Car Body"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Vehicles/F1/F1_car_LowPoly.F1_car_LowPoly'"));
	if (MeshContainer.Succeeded())
	{
		body->SetSkeletalMesh(MeshContainer.Object);
	}

	SetRootComponent(body);

	SpringArmComp->SetupAttachment(body);

	CameraTP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTP"));
	CameraTP->SetupAttachment(SpringArmComp);

	CameraFP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFP"));
	CameraFP->SetupAttachment(SpringArmComp);
}

// Called when the game starts or when spawned
void ACar::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

