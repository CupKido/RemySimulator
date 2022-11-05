// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Test3/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"

#include "RemyCharacter.generated.h"


UCLASS()
class TEST3_API ARemyCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ARemyCharacter();

	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	void PlayFireMontage(bool bAiming);
	void PlayElimMontage();

	void Elim();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim();
	virtual void Destroyed() override;
protected:
	virtual void BeginPlay() override;


	 
	//Input funcs
	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();

	// Play Montages

	void PlayHitReactMontage();

	// Health Actions
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor * DamageCauser);
	void UpdateHUDHealth();
	// poll for any relevant class and initialize HUD
	void PollInit();

	void ZoomInCamera();
	void ZoomOutCamera();
	void SprintPressed();
	void SpeedPressed();


private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"));
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon * LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* Combat;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	UFUNCTION(Server, Reliable)
	void ServerSpeedPressed(bool ToSpeed);

	UPROPERTY(EditAnywhere)
	int SuperSpeed;

	float AO_Yaw;
	float AO_Pitch;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	/** 
	* Player Health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();

	class ARemyPlayerController* RemyPlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 2.2f;

	void ElimTimerFinished();

	/** 
	*	Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;

	FOnTimelineFloat DissolveTrack;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	UFUNCTION()
	void StartDissolve();

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	void SetDynamicScalarParameterValueDissolve(UMaterialInstanceDynamic* DynamicDissolveMaterialInstance, float DissolveValue);

	void SetDynamicDissolveMatInstance(UMaterialInstanceDynamic* DynamicDissolveMaterialInstance, int index);

	//instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceShoe;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceTop;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceBottom;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceHair;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceBody;

	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstanceEyelash;


	//instance set on blueprint, used with dynamic instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstanceShoe;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstanceTop;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstanceBottom;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstanceHair;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstanceBody;

	UPROPERTY(EditAnywhere, Category = Elim)
		UMaterialInstance* DissolveMaterialInstanceEyelash;

	/** 
	* Elim bot
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	class ARemyPlayerState * RemyPlayerState;

public:
	void SetOverlappingWeapon(AWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();
	AWeapon* GetEquippedWeapon();
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FVector GetHitTarget() const;
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
};
