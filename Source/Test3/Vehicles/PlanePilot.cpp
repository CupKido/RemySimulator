// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanePilot.h"

// Sets default values
APlanePilot::APlanePilot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->TargetArmLength = 800.f;

	Fuselage = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fuselage"));
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshContainer(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_fuselage.SM_fuselage'"));
	if (MeshContainer.Succeeded())
	{
		Fuselage->SetStaticMesh(MeshContainer.Object);
	}

	SetRootComponent(Fuselage);

	SpringArmComp->SetupAttachment(Fuselage);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);

	Glass = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Glass"));
	AileronL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AileronL"));
	AileronR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AileronR"));
	RudderL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RudderL"));
	RudderR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RudderR"));
	ElevatorL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorL"));
	ElevatorR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ElevatorR"));
	FlapsL = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlapsL"));
	FlapsR = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlapsR"));

	Fuselage->bEditableWhenInherited = true;
	Glass->bEditableWhenInherited = true;
	AileronL->bEditableWhenInherited = true;
	AileronR->bEditableWhenInherited = true;
	RudderL->bEditableWhenInherited = true;
	RudderR->bEditableWhenInherited = true;
	ElevatorL->bEditableWhenInherited = true;
	ElevatorR->bEditableWhenInherited = true;
	FlapsL->bEditableWhenInherited = true;
	FlapsR->bEditableWhenInherited = true;


	Glass->SetupAttachment(Fuselage, TEXT("Glass"));
	AileronL->SetupAttachment(Fuselage, TEXT("AileronL"));
	AileronR->SetupAttachment(Fuselage, TEXT("AileronR"));
	RudderL->SetupAttachment(Fuselage, TEXT("RudderL"));
	RudderR->SetupAttachment(Fuselage, TEXT("RudderR"));
	ElevatorL->SetupAttachment(Fuselage, TEXT("ElevatorL"));
	ElevatorR->SetupAttachment(Fuselage, TEXT("ElevatorR"));
	FlapsL->SetupAttachment(Fuselage, TEXT("FlapsL"));
	FlapsR->SetupAttachment(Fuselage, TEXT("FlapsR"));

	
}

// Called when the game starts or when spawned
void APlanePilot::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlanePilot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PrintVariables();
}

// Called to bind functionality to input
void APlanePilot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void APlanePilot::PrintVariables()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("thrust Speed: %f"), thrustSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("current Speed: %f"), currentSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("applied Gravity: %f"), appliedGravity));
}


