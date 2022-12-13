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

	AimOffset(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
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


	PlayerInputComponent->BindAction("SpawnVehicle", IE_Pressed, this, &ARemyCharacter::SpawnVehicle);
	PlayerInputComponent->BindAction("ZoomInCamera", IE_Pressed, this, &ARemyCharacter::ZoomInCamera);
	PlayerInputComponent->BindAction("ZoomOutCamera", IE_Pressed, this, &ARemyCharacter::ZoomOutCamera);
	PlayerInputComponent->BindAction("SuperSpeed", IE_Pressed, this, &ARemyCharacter::SpeedPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ARemyCharacter::SprintPressed);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ARemyCharacter::SprintPressed);

	PlayerInputComponent->BindAction("EnterVehicle", IE_Pressed, this, &ARemyCharacter::EnterVehicleButtonPressed);
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ARemyCharacter::Interact);


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
	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();
	if (RemyPlayerController) {
		DisableInput(RemyPlayerController);
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

void ARemyCharacter::ReloadButtonPressed()
{
	if (Combat) {
		Combat->Reload();
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

void ARemyCharacter::EnterVehicleButtonPressed()
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
	DOREPLIFETIME(ARemyCharacter, Health);
	DOREPLIFETIME(ARemyCharacter, OverlappedPlane);
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
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString("Hit"));
	}
	UpdateHUDHealth();
	PlayHitReactMontage();
	RegainControl();
	if (Health < 0.5f && !bElimmed) {
		ARemyGameMode* RemyGameMode = GetWorld()->GetAuthGameMode<ARemyGameMode>();
		if (RemyGameMode) {
			RemyPlayerController = RemyPlayerController == nullptr ? Cast<ARemyPlayerController>(Controller) : RemyPlayerController;
			ARemyPlayerController* AttackerController = Cast<ARemyPlayerController>(InstigatorController);
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

FVector ARemyCharacter::GetHitTarget() const {
	if (Combat == nullptr) return FVector();

	return Combat->HitTarget;
}

ECombatState ARemyCharacter::GetCombatState() const
{

	if (Combat == nullptr) return ECombatState::ECS_MAX;

	return Combat->CombatState;

}

void ARemyCharacter::SpawnVehicle() {
	if (!HasAuthority()) {
		ServerSpawnVehicle();
		return;
	}
	APawn* InstigatorPawn = Cast<APawn>(this);

	if (VehicleClass && InstigatorPawn) {
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = InstigatorPawn;
		UWorld* World = GetWorld();
		if (World) {
			ControlledVehicle = World->SpawnActor<APawn>(
				VehicleClass,
				GetActorLocation() + FVector(0,0,300),
				GetActorRotation(),
				SpawnParams
				);
			if (ControlledVehicle) {
				
				GetCharacterMovement()->StopMovementImmediately();
				GetController()->Possess(ControlledVehicle);
				APlanePilot* temp = Cast<APlanePilot>(ControlledVehicle);
				if (temp) {
					temp->SetEquippedCharacter(this);
				}
			}
		}
	}



}

void ARemyCharacter::ServerSpawnVehicle_Implementation() {
	SpawnVehicle();
}

void ARemyCharacter::RegainControl() {
	if (ControlledVehicle) {
		ControlledVehicle->Destroy();
	}
}

void ARemyCharacter::SetOverlappingPlane(APlanePilot* Plane) {
	OverlappedPlane = Plane;
}

void ARemyCharacter::OnRep_OverlappingPlane(APlanePilot* LastPlane) {
	if (OverlappedPlane) {
		int a = 1;
	}
	if (LastPlane)
	{
		// Hide Widget
	}
}

void ARemyCharacter::Interact() {
	GetMesh()->SetVisibility(false);
	if (OverlappedPlane) {
		//EnterPlane();
		ServerEnterPlane();
	}
}

void ARemyCharacter::EnterPlane()
{
	if (OverlappedPlane && !OverlappedPlane->GetIsEquipped()) {
		
		GetCharacterMovement()->StopMovementImmediately();
		OverheadWidget->SetVisibility(false);
		if (GetController() && HasAuthority()) {
			OverlappedPlane->SetEquippedCharacter(this);
			GetController()->Possess(OverlappedPlane);
		}
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon) {
			Combat->EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
		}
	}
}

void ARemyCharacter::ServerEnterPlane_Implementation() {
	MulticastEnterPlane();
}

void ARemyCharacter::MulticastEnterPlane_Implementation() {
	EnterPlane();
}


void ARemyCharacter::ExitVehicle(FVector newLocation) {
	
	OverheadWidget->SetVisibility(true);
	if (GetCapsuleComponent() && GetMesh()) {
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		GetMesh()->SetVisibility(true);
	}
	if (Combat && Combat->EquippedWeapon) {
		Combat->EquippedWeapon->GetWeaponMesh()->SetVisibility(false);
	}
	if (HasAuthority()) {
		SetActorLocation(newLocation);
	}
}