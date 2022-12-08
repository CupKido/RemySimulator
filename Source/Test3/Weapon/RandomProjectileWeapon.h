// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileWeapon.h"
#include "RandomProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class TEST3_API ARandomProjectileWeapon : public AProjectileWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass0;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass1;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass2;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass3;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass4;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass5;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass6;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass7;

	UPROPERTY(EditAnywhere)
	uint32 NumOfProjectiles;
};
