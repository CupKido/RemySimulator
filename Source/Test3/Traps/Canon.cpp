// Fill out your copyright notice in the Description page of Project Settings.


#include "Canon.h"
#include "Engine/EngineTypes.h"
#include <Runtime/Engine/Classes/Kismet/KismetMathLibrary.h>
#include "Test3/Interfaces/FollowInterface.h"

// Sets default values
ACanon::ACanon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	GunMechineMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GunMechineMesh"));
	GunMechineMesh->SetupAttachment(Root);

	TCanon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Canon"));
	TCanon->SetupAttachment(GunMechineMesh, TEXT("CanonSocket"));

	Target1 = CreateDefaultSubobject<USceneComponent>(TEXT("Target1"));
	Target1->SetupAttachment(Root);

	Target2 = CreateDefaultSubobject<USceneComponent>(TEXT("Target2"));
	Target2->SetupAttachment(Root);

	FollowTarget = CreateDefaultSubobject<USceneComponent>(TEXT("FollowTarget"));
	FollowTarget->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void ACanon::BeginPlay()
{
	Super::BeginPlay();
	
	GetWorldTimerManager().SetTimer(TimeHandler, this, &ACanon::ChangeCanonTarget, 5.f, true, 1.f);
}

// Called every frame
void ACanon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLookAt();
}

void ACanon::UpdateLookAt()
{
	FVector Start = GunMechineMesh->GetSocketLocation(TEXT("CanonSocket"));
	FVector End = FollowTarget->GetComponentLocation();
	FRotator LookAtRotatin = UKismetMathLibrary::FindLookAtRotation(Start, End);

	if (GunMechineMesh->GetAnimInstance()->Implements<UFollowInterface>())
	{
		IFollowInterface::Execute_UpdateLookAt(GunMechineMesh->GetAnimInstance(), LookAtRotatin);
	}
}

void ACanon::ChangeCanonTarget()
{
	TimerCounter++;

	if (TimerCounter % 2 == 0)
	{
		FollowTarget->SetWorldLocation(Target1->GetComponentLocation());
	}
	else
	{
		FollowTarget->SetWorldLocation(Target2->GetComponentLocation());
	}
}


