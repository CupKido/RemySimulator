// Fill out your copyright notice in the Description page of Project Settings.


#include "Car.h"
#include "Math/Rotator.h"


/*
* Calculations for animating the wheels.
* front rot angle = (RadianToDegrees((Speed/FrontRadius)* DeltaTime * -1) + FrontRotAngle) % 360
* rear rot angle = (RadianToDegrees((Speed/RearRadius)* DeltaTime * -1) + RearRotAngle) % 360
*/



// Sets default values
ACar::ACar()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->TargetArmLength = 600.f;

	body = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Car Body"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshContainer(TEXT("SkeletalMesh'/Game/Vehicles/F1/F1_car_LowPoly.F1_car_LowPoly'"));
	if (MeshContainer.Succeeded())
	{
		body->SetSkeletalMesh(MeshContainer.Object);
	}

	SetRootComponent(body);

	SpringArmComp->SetupAttachment(body);
	SpringArmComp->AddRelativeRotation(FRotator(-15,0,0));
	SpringArmComp->AddRelativeLocation(FVector(0,0,80));

	CameraTP = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTP"));
	CameraTP->SetupAttachment(SpringArmComp);

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

	this->AddActorWorldOffset(GetActorForwardVector() * CurrentSpeed * DeltaTime, true);
	this->AddActorWorldRotation(FRotator(0, 
		CurrentSpeed * CurrentSteeringAngle * SpeedToRotate * FApp::GetDeltaTime() * (CurrentSpeed < 0.0 ? 2 : 1), 0));
	
}

// Called to bind functionality to input
void ACar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	PlayerInputComponent->BindAxis("Throttle", this, &ACar::Throttle);
	PlayerInputComponent->BindAxis("Steer", this, &ACar::Steer);
	PlayerInputComponent->BindAxis("LookRight", this, &ACar::LookRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ACar::LookUp);

	//PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ACar::EquipButtonPressed);
}


void ACar::Throttle(float Value) {
	if (Value) {
		CurrentSpeed = FMath::FInterpTo(CurrentSpeed, MaxSpeed * Value, FApp::GetDeltaTime(), 4);

		check(GEngine != nullptr) {
			FString Path = "Movement Speed is " + FString::SanitizeFloat(CurrentSpeed);

			const TCHAR* CPath = *Path;

			// Display a debug message for five seconds. 
		   // The -1 "Key" value (first argument) indicates that we will never need to update or refresh this message.
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Blue, CPath);
		}
	}
}
void ACar::Steer(float Value) {
	if (Value) {
		CurrentSteeringAngle = FMath::FInterpTo(CurrentSteeringAngle, MaxSteeringAngle * Value, FApp::GetDeltaTime(), 4);

		check(GEngine != nullptr) {
			FString Path = "Steering angle is " + FString::SanitizeFloat(CurrentSteeringAngle);

			const TCHAR* CPath = *Path;

			// Display a debug message for five seconds. 
		   // The -1 "Key" value (first argument) indicates that we will never need to update or refresh this message.
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, CPath);
		}
	}
}
void ACar::LookRight(float Value) {

}
void ACar::LookUp(float Value) {

}

