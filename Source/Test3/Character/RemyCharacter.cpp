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
#include "Test3/Test3.h"
#include "Test3/PlayerController/RemyPlayerController.h"
#include "Test3/GameMode/RemyGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Test3/PlayerState/RemyPlayerState.h"
#include "Test3/Weapon/WeaponTypes.h"
#include "GameFramework/PlayerController.h"


#include "Test3/HUD/OverheadWidget.h"
#include "Internationalization/Text.h"
// Sets default values
ARemyCharacter::ARemyCharacter()
{

	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 500.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->SetActive(true);

	KilledCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("KilledCameraBoom"));
	KilledCameraBoom->SetupAttachment(GetMesh());
	KilledCameraBoom->TargetArmLength = 500.f;
	KilledCameraBoom->bUsePawnControlRotation = false;

	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true);


	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	SuperSpeed = 120000;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
}

void ARemyCharacter::Destroyed() {
	Super::Destroyed();

	if (ElimBotComponent) {
		ElimBotComponent->DestroyComponent();
	}

	ARemyGameMode* RemyGameMode = Cast<ARemyGameMode>(UGameplayStatics::GetGameMode(this));
	bool bMatchNotInProgress = RemyGameMode && RemyGameMode->GetMatchState() != MatchState::InProgress;

	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress) {
		Combat->EquippedWeapon->Destroy();
	}
}



void ARemyCharacter::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();
	if (HasAuthority()) {
		OnTakeAnyDamage.AddDynamic(this, &ARemyCharacter::ReceiveDamage);
	}

}

void ARemyCharacter::UpdateHUDHealth()
{
	RemyPlayerController = RemyPlayerController == nullptr ? Cast<ARemyPlayerController>(Controller) : RemyPlayerController;
	if (RemyPlayerController) {
		RemyPlayerController->SetHUDHealth(Health, MaxHealth);
	}
	/*if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Green, RemyPlayerController ? TEXT("t") : TEXT("f"));
	}*/
}


void ARemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ARemyCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationPitch = false;
		return;
	}
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
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ARemyCharacter::ReloadButtonPressed);

	if (EnableCameraZoom) {
		PlayerInputComponent->BindAction("ZoomInCamera", IE_Pressed, this, &ARemyCharacter::ZoomInCamera);
		PlayerInputComponent->BindAction("ZoomOutCamera", IE_Pressed, this, &ARemyCharacter::ZoomOutCamera);
	}
	
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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage) {
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ARemyCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage) {
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType()) {
		case EWeaponType::EWT_AssultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("Rifle");
			break;
		}
		AnimInstance->Montage_JumpToSection(SectionName);
	}


}

void ARemyCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage) {
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ARemyCharacter::Elim()
{
	if (Combat && Combat->EquippedWeapon) {
		Combat->EquippedWeapon->Dropped();
	}
	MulticastElim();
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ARemyCharacter::ElimTimerFinished,
		ElimDelay
	);
}


void ARemyCharacter::MulticastElim_Implementation()
{
	if (RemyPlayerController)
	{
		RemyPlayerController->SetHUDWeaponAmmo(0);
	}
	bElimmed = true;
	PlayElimMontage();

	// Start Dissolve effect
	bool InstancesAreOK = DissolveMaterialInstanceShoe && DissolveMaterialInstanceTop && DissolveMaterialInstanceBottom && DissolveMaterialInstanceHair && DissolveMaterialInstanceBody && DissolveMaterialInstanceEyelash;
	if (InstancesAreOK) {
		DynamicDissolveMaterialInstanceShoe = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceShoe, this);
		DynamicDissolveMaterialInstanceTop = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceTop, this);
		DynamicDissolveMaterialInstanceBottom = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceBottom, this);
		DynamicDissolveMaterialInstanceHair = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceHair, this);
		DynamicDissolveMaterialInstanceBody = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceBody, this);
		DynamicDissolveMaterialInstanceEyelash = UMaterialInstanceDynamic::Create(DissolveMaterialInstanceEyelash, this);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceShoe, 0);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceTop, 1);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceBottom, 2);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceHair, 3);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceBody, 4);
		SetDynamicDissolveMatInstance(DynamicDissolveMaterialInstanceEyelash, 5);
	}

	StartDissolve();

	// Disable character movement
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	//Disable Collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn Elim Bot
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint = GetActorLocation() + FVector(0, 0, 200);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ElimBotEffect, ElimBotSpawnPoint, GetActorRotation());

	}
	if (ElimBotSound) {
		UGameplayStatics::SpawnSoundAtLocation(this, ElimBotSound, GetActorLocation());
	}
	bool bHideSniperScope = IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope) {
		ShowSniperScopeWidget(false);
	}
	if (OverheadWidget) {
		OverheadWidget->SetVisibility(false);
	}
}

void ARemyCharacter::SetDynamicDissolveMatInstance(UMaterialInstanceDynamic* DynamicDissolveMaterialInstance, int index) {
	GetMesh()->SetMaterial(index, DynamicDissolveMaterialInstance);
	DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
	DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);

}

void ARemyCharacter::ElimTimerFinished()
{
	ARemyGameMode* RemyGameMode = GetWorld()->GetAuthGameMode<ARemyGameMode>();
	if (RemyGameMode) {
		RemyGameMode->RequestRespawn(this, Controller);
	}
	bElimmed = false;
}

void ARemyCharacter::PlayHitReactMontage()
{

	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage) {
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}

}


//InputAction

void ARemyCharacter::MoveForward(float Value) {
	if (bDisableGameplay) return;
	if (Controller != nullptr && Value != 0) {
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, Value);
	}
}

void ARemyCharacter::MoveRight(float Value) {
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Crouch();
	}
}

void ARemyCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->Reload();
	}
}

void ARemyCharacter::AimButtonPressed() {
	if (bDisableGameplay) return;
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
	if (bDisableGameplay) return;
	if (bIsCrouched) {
		UnCrouch();
	}
	else {
		Super::Jump();
	}
}

void ARemyCharacter::FireButtonPressed() {
	if (bDisableGameplay) return;
	if (Combat) {
		Combat->FireButtonPressed(true);
	}
}

void ARemyCharacter::FireButtonReleased() {
	if (bDisableGameplay) return;
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
	if (GetCharacterMovement()->MaxWalkSpeed != 900) {
		GetCharacterMovement()->MaxWalkSpeed = 900;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = 500;
	}
}

void ARemyCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ARemyCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ARemyCharacter, Health);
	DOREPLIFETIME(ARemyCharacter, bDisableGameplay);
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

void ARemyCharacter::ChangeDeathCamera(AActor* Camera, AController* controller)
{
	ARemyPlayerController* AttackerController = Cast<ARemyPlayerController>(controller);

	RemyPlayerController->SetViewTargetWithBlend(Camera, 0.5f);

}

void ARemyCharacter::ChangeDeathCamera(UCameraComponent* Camera, AController* controller)
{
		
}

void ARemyCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold) {
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else {
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh()) {
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ARemyCharacter::PollInit() {
	if (RemyPlayerState == nullptr) {
		RemyPlayerState = GetPlayerState<ARemyPlayerState>();
		if (RemyPlayerState) {
			RemyPlayerState->AddToScore(0.f);
			RemyPlayerState->AddToDefeats(0);
		}
	}
	if (bShowUsernameOverHead && !bUsernameHasSet) {
		if (OverheadWidget) {
			UOverheadWidget* overheadWidget = Cast<UOverheadWidget>(OverheadWidget->GetWidget());
			if (overheadWidget && GetPlayerState()) {
				overheadWidget->ShowPlayerName(this);
				bUsernameHasSet = true;
			}
		}
	}
}

void ARemyCharacter::UpdateDissolveMaterial(float DissolveValue)
{

	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceShoe, DissolveValue);
	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceTop, DissolveValue);
	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceBottom, DissolveValue);
	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceHair, DissolveValue);
	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceBody, DissolveValue);
	SetDynamicScalarParameterValueDissolve(DynamicDissolveMaterialInstanceEyelash, DissolveValue);

}

void ARemyCharacter::SetDynamicScalarParameterValueDissolve(UMaterialInstanceDynamic* DynamicDissolveMaterialInstance, float DissolveValue) {
	/*if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Blue, DynamicDissolveMaterialInstance ? TEXT("Dissolving") : TEXT("not Dissolving"));
	}*/
	if (DynamicDissolveMaterialInstance) {
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ARemyCharacter::StartDissolve()
{

	DissolveTrack.BindDynamic(this, &ARemyCharacter::UpdateDissolveMaterial);

	if (DissolveCurve && DissolveTimeline) {
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
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

void ARemyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{

	if (DamagedActor == DamageCauser) return;
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();

	PlayHitReactMontage();
	if (Health < 0.5f && !bElimmed) {
		ARemyGameMode* RemyGameMode = GetWorld()->GetAuthGameMode<ARemyGameMode>();
		if (RemyGameMode) {
			RemyPlayerController = RemyPlayerController == nullptr ? Cast<ARemyPlayerController>(Controller) : RemyPlayerController;
			ARemyPlayerController* AttackerController = Cast<ARemyPlayerController>(InstigatorController);
			ARemyCharacter* RemyCauser = Cast<ARemyCharacter>(AttackerController->GetPawn());
			if (RemyCauser) {
				/*ChangeDeathCamera(RemyCauser->CameraBoom, this);*/
				ChangeDeathCamera(RemyCauser, InstigatorController);
				if (GEngine) {
					GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString("Valid"));
				}
			}
			RemyGameMode->PlayerEliminated(this, RemyPlayerController, AttackerController);
		}

	}

}

void ARemyCharacter::OnRep_Health()
{
	UpdateHUDHealth();
	PlayHitReactMontage();

}

void ARemyCharacter::ZoomInCamera()
{
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength - 40, 70, 800);
	/*
	CameraBoom->SocketOffset = FVector(FMath::Clamp(CameraBoom->SocketOffset.X + 2, 0, 8), 0, FMath::Clamp(CameraBoom->SocketOffset.Z - 8, 40, 150));*/
}
void ARemyCharacter::ZoomOutCamera()
{
	CameraBoom->TargetArmLength = FMath::Clamp(CameraBoom->TargetArmLength + 40, 70, 800);/*
	CameraBoom->SocketOffset = FVector(FMath::Clamp(CameraBoom->SocketOffset.X - 2, 0, 8), 0, FMath::Clamp(CameraBoom->SocketOffset.Z + 8, 40, 150));*/
}

AWeapon* ARemyCharacter::GetEquippedWeapon()
{
	if (Combat == nullptr) return nullptr;
	return Combat->EquippedWeapon;
}

FVector ARemyCharacter::GetHitTarget() const {
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
}

ECombatState ARemyCharacter::GetCombatState() const
{

	if (Combat == nullptr) return ECombatState::ECS_MAX;

	return Combat->CombatState;

}

void ARemyCharacter::SetBoomOffsetAndLength(FVector Offset, float Length) {
	CameraBoom->TargetArmLength = FMath::Clamp(Length, 70, 800);
	
	CameraBoom->SocketOffset = Offset;
}