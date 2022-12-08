// Fill out your copyright notice in the Description page of Project Settings.


#include "RandomProjectileWeapon.h"

void ARandomProjectileWeapon::Fire(const FVector& HitTarget)
{
	uint32 Selection = FMath::RandRange(0, NumOfProjectiles - 1);

	switch (Selection) {
	default:
	case 0:
		ProjectileClass = ProjectileClass0;
		break;
	case 1:
		ProjectileClass = ProjectileClass1;
		break;
	case 2:
		ProjectileClass = ProjectileClass2;
		break;
	case 3:
		ProjectileClass = ProjectileClass3;
		break;
	case 4:
		ProjectileClass = ProjectileClass4;
		break;
	case 5:
		ProjectileClass = ProjectileClass5;
		break;
	case 6:
		ProjectileClass = ProjectileClass6;
		break;
	case 7:
		ProjectileClass = ProjectileClass7;
		break;
	}

	Super::Fire(HitTarget);
}
