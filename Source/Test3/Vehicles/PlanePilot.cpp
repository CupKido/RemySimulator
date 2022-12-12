// Fill out your copyright notice in the Description page of Project Settings.


#include "PlanePilot.h"


// Sets default values
APlanePilot::APlanePilot()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->TargetArmLength = 1000.f;

	Fuselage = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Fuselage"));
	
	/*static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshContainer(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_fuselage.SM_fuselage'"));
	if (MeshContainer.Succeeded())
		Fuselage->SetStaticMesh(MeshContainer.Object);*/

	SetRootComponent(Fuselage);

	SpringArmComp->SetupAttachment(Fuselage);

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp);
	SpringArmComp->SocketOffset = FVector(0, 0, 300);
	

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

	// attach meshes to root component
	Glass->SetupAttachment(Fuselage, TEXT("Glass"));
	AileronL->SetupAttachment(Fuselage, TEXT("AileronL"));
	AileronR->SetupAttachment(Fuselage, TEXT("AileronR"));
	RudderL->SetupAttachment(Fuselage, TEXT("RudderL"));
	RudderR->SetupAttachment(Fuselage, TEXT("RudderR"));
	ElevatorL->SetupAttachment(Fuselage, TEXT("ElevatorL"));
	ElevatorR->SetupAttachment(Fuselage, TEXT("ElevatorR"));
	FlapsL->SetupAttachment(Fuselage, TEXT("FlapsL"));
	FlapsR->SetupAttachment(Fuselage, TEXT("FlapsR"));

	// load meshes
	/*static ConstructorHelpers::FObjectFinder<UStaticMesh> GlassMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_glass.SM_glass'"));
	if (GlassMesh.Succeeded())
		Glass->SetStaticMesh(GlassMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AileronLMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_aileronL.SM_aileronL'"));
	if (AileronLMesh.Succeeded())
		AileronL->SetStaticMesh(AileronLMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> AileronRMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_aileronR.SM_aileronR'"));
	if (AileronRMesh.Succeeded())
		AileronR->SetStaticMesh(AileronRMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RudderLMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_rudderL.SM_rudderL'"));
	if (RudderLMesh.Succeeded())
		RudderL->SetStaticMesh(RudderLMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> RudderRMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_rudderR.SM_rudderR'"));
	if (RudderRMesh.Succeeded())
		RudderR->SetStaticMesh(RudderRMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorLMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_aileronL.SM_aileronL'"));
	if (ElevatorLMesh.Succeeded())
		ElevatorL->SetStaticMesh(ElevatorLMesh.Object);
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> ElevatorRMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_aileronR.SM_aileronR'"));
	if (ElevatorRMesh.Succeeded())
		ElevatorR->SetStaticMesh(ElevatorRMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlapsLMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_flapsL.SM_flapsL'"));
	if (FlapsLMesh.Succeeded())
		FlapsL->SetStaticMesh(FlapsLMesh.Object);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> FlapsRMesh(TEXT("StaticMesh'/Game/Vehicles/Plane/Assets/SM_flapsR.SM_flapsR'"));
	if (FlapsRMesh.Succeeded())
		FlapsR->SetStaticMesh(FlapsRMesh.Object);*/
	
	TextToEnter = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Press F to Enter"));
}

// Called when the game starts or when spawned
void APlanePilot::BeginPlay()
{
	Super::BeginPlay();

	thrustSpeed = minThrustSpeed;
	currentSpeed = minThrustSpeed;

	if (ThrusterSystem) {
		//     This spawns the chosen effect on the owning WeaponMuzzle SceneComponent
		//UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(ThrusterSystem, /*USceneComponent*/0, NAME_None, FVector(0.f), FRotator(0.f), EAttachLocation::Type::KeepRelativeOffset, true);
		//     Parameters can be set like this (see documentation for further info) - the names and type must match the user exposed parameter in the Niagara System
		//NiagaraComp->SetNiagaraVariableFloat(FString("StrengthCoef"), /*float*/0.0);
	}

}

// Called every frame
void APlanePilot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdatePosition(DeltaTime);
	PrintVariables();
}

// Called to bind functionality to input
void APlanePilot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("Thrust", this, &APlanePilot::Thrust);
	PlayerInputComponent->BindAxis("Yaw", this, &APlanePilot::Turn);
	PlayerInputComponent->BindAxis("Pitch", this, &APlanePilot::Pitch);
	PlayerInputComponent->BindAxis("Roll", this, &APlanePilot::Roll);
}

void APlanePilot::UpdatePosition(float DeltaSeconds)
{
	//calculate current spped (possible in one line but it was too cluttered)
	if (thrustSpeed < currentSpeed)
		currentSpeed = FMath::FInterpTo(currentSpeed, thrustSpeed, DeltaSeconds, drag);
	else
		currentSpeed = thrustSpeed;

	// calculate new position
	FVector newPosition = currentSpeed* DeltaSeconds* GetActorForwardVector();

	// calculate applied gravity
	appliedGravity = FMath::GetMappedRangeValueClamped(FVector2D(0.0, minThrustSpeed), FVector2D(gravity, 0.0), currentSpeed);

	//update position
	newPosition.Z = newPosition.Z - appliedGravity * DeltaSeconds;
	this->AddActorWorldOffset(newPosition, true);

}

void APlanePilot::UpdateYaw(float Value, float DeltaSeconds)
{
	targetYaw = Value;
	currentYaw = FMath::FInterpTo(currentYaw, targetYaw, DeltaSeconds, 10.0);

	this->AddActorLocalRotation(FRotator(0, currentYaw * DeltaSeconds * 20.0, 0), true);

	if (RudderL) {
		RudderL->SetRelativeRotation(FRotator(0, FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxRudderYaw, -maxRudderYaw), currentYaw), 0));
	}
	if (RudderR) {
		RudderR->SetRelativeRotation(FRotator(0, FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxRudderYaw, -maxRudderYaw), currentYaw), 0));
	}
	
}

void APlanePilot::UpdatePitch(float Value, float DeltaSeconds)
{
	targetPitch = Value;
	currentPitch = FMath::FInterpTo(currentPitch, targetPitch, DeltaSeconds, 10.0);

	this->AddActorLocalRotation(FRotator(currentPitch * DeltaSeconds * 20.0, 0 ,0), true);


	/*FlapsL->SetRelativeRotation(FRotator(FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxFlapPitch, -maxFlapPitch), currentPitch), 0, 0));
	FlapsR->SetRelativeRotation(FRotator(FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxFlapPitch, -maxFlapPitch), currentPitch), 0, 0));

	ElevatorR->SetRelativeRotation(FRotator(FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxElevatorPitch, -maxElevatorPitch), currentPitch), 0, 0));
	ElevatorL->SetRelativeRotation(FRotator(FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxElevatorPitch, -maxElevatorPitch), currentPitch), 0, 0));*/
}

void APlanePilot::UpdateRoll(float Value, float DeltaSeconds)
{
	targetRoll= Value;
	currentRoll = FMath::FInterpTo(currentRoll, targetRoll, DeltaSeconds, 10.0);

	this->AddActorLocalRotation(FRotator(0, 0, currentRoll * DeltaSeconds * 20.0), true);


	/*AileronL->SetRelativeRotation(FRotator(-1.0 * FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxAileronPitch, -maxAileronPitch), currentRoll),0 ,0));
	AileronR->SetRelativeRotation(FRotator(FMath::GetMappedRangeValueClamped(FVector2D(-1, 1), FVector2D(maxAileronPitch, -maxAileronPitch), currentRoll), 0, 0));*/
}

void APlanePilot::PrintVariables()
{
	GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("height: %f"), this->GetActorLocation().Z));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("thrust Speed: %f"), thrustSpeed));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("current Speed: %f"), currentSpeed));
	//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::Printf(TEXT("applied Gravity: %f"), appliedGravity));
}

void APlanePilot::Thrust(float Value)
{
	thrustSpeed = FMath::Clamp(Value* thrustMultiplier* FApp::GetDeltaTime() + thrustSpeed, 0, maxThrustSpeed);
}

void APlanePilot::Turn(float Value)
{
	UpdateYaw(Value, FApp::GetDeltaTime());
}

void APlanePilot::Pitch(float Value)
{
	UpdatePitch(Value, FApp::GetDeltaTime());
}

void APlanePilot::Roll(float Value)
{
	UpdateRoll(Value, FApp::GetDeltaTime());
}


