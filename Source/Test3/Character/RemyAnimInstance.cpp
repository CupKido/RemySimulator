// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyAnimInstance.h"
#include "RemyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Test3/Weapon/Weapon.h"
#include "Test3/RemyTypes/CombatState.h"

void URemyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	RemyC = Cast<ARemyCharacter>(TryGetPawnOwner());
}


void URemyAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (RemyC == nullptr)
	{
		RemyC = Cast<ARemyCharacter>(TryGetPawnOwner());
	}
	if (RemyC == nullptr) return;

	FVector Velocity = RemyC->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = RemyC->GetCharacterMovement()->IsFalling();
	bIsAccelerating = RemyC->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;

	bWeaponEquipped = RemyC->IsWeaponEquipped();
	EquippedWeapon = RemyC->GetEquippedWeapon();
	bIsCrouched = RemyC->bIsCrouched;
	bAiming = RemyC->IsAiming();
	bElimmed = RemyC->IsElimmed();

	//OffsetYaw for strafing
	FRotator AimRotation = RemyC->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(RemyC->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;


	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = RemyC->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Pitch = RemyC->GetAO_Pitch();
	
	bUseFabrik = RemyC->GetCombatState() != ECombatState::ECS_Reloading && !RemyC->IsInEmote();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && RemyC->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		RemyC->GetMesh()->TransformToBoneSpace(FName("RightHand"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (RemyC->IsLocallyControlled()) {
			bLocallyControlled = true; //Change to true
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("RightHand"), ERelativeTransformSpace::RTS_World);
			FVector RightHandLocation = RightHandTransform.GetLocation();
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandLocation, RemyC->GetHitTarget());
			LookAtRotation.Add(90, 0 , 0);
			LookAtRotation.Add(0, 0, -90);
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 55.f);
		}
		else {

		}
		

	}

	
}
