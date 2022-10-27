// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Test3/Weapon/Weapon.h"
#include "Test3/RemyComponent/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "RemyAnimInstance.h"

#include "Internationalization/Text.h"
// Sets default values
ARemyCharacter::ARemyCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);
	

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	SuperSpeed = 120000;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

void ARemyCharacter::BeginPlay()
{
	Super::BeginPlay();

}


void ARemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);

}

void ARemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ARemyCharacter::Jump);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARemyCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARemyCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ARemyCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ARemyCharacter::LookUp);

	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ARemyCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ARemyCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ARemyCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ARemyCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ARemyCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ARemyCharacter::FireButtonReleased);


	
	PlayerInputComponent->BindAction("ZoomInCamera", IE_Pressed, this, &ARemyCharacter::ZoomInCamera);
	PlayerInputComponent->BindAction("ZoomOutCamera", IE_Pressed, this, &ARemyCharacter::ZoomOutCamera);
	PlayerInputComponent->BindAction("SuperSpeed", IE_Pressed, this, &ARemyCharacter::SpeedPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ARemyCharacter::SprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ARemyCharacter::SprintPressed);


}

void ARemyCharacter::PostInitializeComponents() {
	Super::PostInitializeComponents();
	if (Combat) {
		Combat->Character = this;
	}
}

void ARemyCharacter::PlayFireMontage(bool bAiming) {
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance * AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
//InputAction

void ARemyCharacter::MoveForward(float Value) {
	if (Controller != nullptr && Value != 0) {
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ARemyCharacter::MoveRight(float Value) {
	if (Controller != nullptr && Value != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(Direction, Value);
	}
}

void ARemyCharacter::Turn(float Value) {
	AddControllerYawInput(Value);
}

void ARemyCharacter::LookUp(float Value) {
	AddControllerPitchInput(Value);
}

void ARemyCharacter::EquipButtonPressed() 
{
	if (Combat)
	{
		if (HasAuthority()) 
		{
		Combat->EquipWeapon(OverlappingWeapon);
		}
		else 
		{
			ServerEquipButtonPressed();
		}
	}
}


void ARemyCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat) {
		Combat->EquipWeapon(OverlappingWeapon);
	}
}

void ARemyCharacter::CrouchButtonPressed() 
{
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Crouch();
	}
}

void ARemyCharacter::AimButtonPressed() {
	if (Combat) {
		Combat->SetAiming(true);
	}
}
void ARemyCharacter::AimButtonReleased() {
	if (Combat) {
		Combat->SetAiming(false);
	}
}

void ARemyCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return;
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled()) {
		// map pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void ARemyCharacter::Jump() {
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void ARemyCharacter::FireButtonPressed() {
	if (Combat) {
		Combat->FireButtonPressed(true);
	}
}

void ARemyCharacter::FireButtonReleased() {
	if (Combat) {
		Combat->FireButtonPressed(false);
	}
}

void ARemyCharacter::SpeedPressed() {
	if (GetCharacterMovement()->MaxWalkSpeed != SuperSpeed) {
		GetCharacterMovement()->MaxWalkSpeed = SuperSpeed;
		ServerSpeedPressed(true);
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = Combat->BaseWalkSpeed;
		ServerSpeedPressed(false);
	}
}

void ARemyCharacter::ServerSpeedPressed_Implementation(bool ToSpeed) {
	if (ToSpeed) {
		GetCharacterMovement()->MaxWalkSpeed = SuperSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = Combat->BaseWalkSpeed;
	}
}

void ARemyCharacter::SprintPressed() {
	if (GetCharacterMovement()->MaxWalkSpeed != 700) {
		GetCharacterMovement()->MaxWalkSpeed = 700;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = 500;
	}
}

void ARemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ARemyCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void ARemyCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) {
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(true);
	}
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}


void ARemyCharacter::SetOverlappingWeapon(AWeapon* Weapon) {
	if (OverlappingWeapon) {
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) {
		if (OverlappingWeapon) {
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool  ARemyCharacter::IsWeaponEquipped()
{
	return (Combat && Combat->EquippedWeapon);
}

bool ARemyCharacter::IsAiming() {
	return (Combat && Combat->bAiming);
}

void ARemyCharacter::ZoomInCamera()
{
	if (CameraBoom->TargetArmLength > 70) {
		CameraBoom->TargetArmLength -= 40;
	}
	else {
		CameraBoom->TargetArmLength = 70;
	}
}
void ARemyCharacter::ZoomOutCamera()
{
	if (CameraBoom->TargetArmLength < 800) {
		CameraBoom->TargetArmLength += 40;
	}
	else {
		CameraBoom->TargetArmLength = 800;
	}
}

AWeapon* ARemyCharacter::GetEquippedWeapon() 
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}