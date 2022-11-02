// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Test3/Weapon/Weapon.h"
#include "Test3/Character/RemyCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h" 
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Test3/PlayerController/RemyPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"



#include "Internationalization/Text.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 500.f;
	AimWalkSpeed = 350.f;
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character) {
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;

		if (Character->GetFollowCamera()) {
			DefaultFOV = Character->GetFollowCamera()->FieldOfView;
			CurrentFOV = DefaultFOV;
		}
	}
	
	
}

// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (Character && Character->IsLocallyControlled()) 
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}

	

}

void UCombatComponent::FireButtonPressed(bool bPressed) {
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed) {
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (GEngine) {
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, bCanFire ? TEXT("true") : TEXT("false"));
	}
	if (bCanFire) {
		bCanFire = false;
		ServerFire(HitTarget);

		if (EquippedWeapon) {
			CrosshairShootingFactor = 0.75f;
		}
		StartFireTimer();
	}
}

void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) {
		bCanFire = true;
		return;
	}
	Character->GetWorldTimerManager().SetTimer(
		FireTimer, this, &UCombatComponent::FireTimerFinished, EquippedWeapon->FireDelay
	);
}

void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed && EquippedWeapon->bAutomatic) {
		Fire();
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget) {
	if (EquippedWeapon == nullptr) return;
	if (Character) {
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming) {
	bAiming = bIsAiming;
	if (EquippedWeapon)
	{
		if (Character) {
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket) {
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
	EquippedWeapon->SetOwner(Character);
	Character->GetCharacterMovement()->bOrientRotationToMovement = false;
	Character->bUseControllerRotationYaw = true;
}

void UCombatComponent::OnRep_EquippedWeapon() {
	if (EquippedWeapon && Character) {
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld) {
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter);
			//DrawDebugSphere(GetWorld(), Start, 16.f, 12, FColor::Red, false);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		if (!GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		)) {
			TraceHitResult.ImpactPoint = End;

		}

		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red;
			bCrosshairOnPerson = true;
		}
		else {
			HUDPackage.CrosshairsColor = FLinearColor::White;
			bCrosshairOnPerson = false;
		}
	}

}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
	if (Controller) {
		HUD = HUD == nullptr ? Cast<ARemyHUD>(Controller->GetHUD()) : HUD;
		if (HUD) {

			if (EquippedWeapon) {
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			}
			else {
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
			}
			// Calculate Crosshair spread

			// [0, 600] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->bIsCrouched ? Character->GetCharacterMovement()->MaxWalkSpeedCrouched : Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			if (Character->GetCharacterMovement()->IsFalling()) {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else {
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming) {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else {
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			if (bCrosshairOnPerson) {
				CrosshairOnPersonFactor = FMath::FInterpTo(CrosshairOnPersonFactor, 0.3f, DeltaTime, 30.f);
			}
			else {
				CrosshairOnPersonFactor = FMath::FInterpTo(CrosshairOnPersonFactor, 0.f, DeltaTime, 20.f);
			}

			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			HUDPackage.CrosshairSpread =
				0.5f +
				CrosshairVelocityFactor +
				CrosshairInAirFactor -
				CrosshairAimFactor +
				CrosshairShootingFactor -
				CrosshairOnPersonFactor;

			HUD->SetHUDPackage(HUDPackage);

		}
	}

}

void UCombatComponent::InterpFOV(float DeltaTime) {
	if (EquippedWeapon == nullptr) return;


	if (Character && Character->GetFollowCamera()) {
		if (bAiming)
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
			/*Character->GetFollowCamera()->PostProcessSettings.bOverride_DepthOfFieldFstop = uint8(32);
			Character->GetFollowCamera()->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = uint8(10000);*/
		}
		else
		{
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
			/*Character->GetFollowCamera()->PostProcessSettings.bOverride_DepthOfFieldFstop = uint8(4);
			Character->GetFollowCamera()->PostProcessSettings.bOverride_DepthOfFieldFocalDistance = uint8(1000);*/
		}
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetAiming(bool bIsAiming) {
	if (EquippedWeapon)
	{
		bAiming = bIsAiming;
		if (Character) {
			Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
		}
		ServerSetAiming(bIsAiming);

	}
}