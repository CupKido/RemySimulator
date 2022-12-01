// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Test3/Character/RemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	//Super::Fire(HitTarget);
	AWeapon::Fire(HitTarget);
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	if (InstigatorPawn == nullptr) return;
	AController* InstigatorController = InstigatorPawn->GetController();
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket) {
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From muzzle/socket to location from TraceUnderCrosshairs
		FVector Start = SocketTransform.GetLocation();

		TMap<ARemyCharacter*, uint32> HitMap;
		for (uint32 i = 0; i < NumberOfPellets; i++) {
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			ARemyCharacter* HitCharacter = Cast<ARemyCharacter>(FireHit.GetActor());
			if (HitCharacter && HasAuthority() && InstigatorController && HitCharacter->GetController() != InstigatorController) {
				if (HitMap.Contains(HitCharacter)) {
					HitMap[HitCharacter]++;
				}
				else {
					HitMap.Emplace(HitCharacter, 1);
				}
			}

			if (ImpactParticles) {
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					ImpactParticles,
					FireHit.ImpactPoint,
					FireHit.ImpactNormal.Rotation()
				);
			}
			if (HitSound) {
				UGameplayStatics::PlaySoundAtLocation(this, HitSound, FireHit.ImpactPoint, 0.7f, FMath::FRandRange(-.5f, .5f));
			}
		}

		for (auto HitPair : HitMap)
		{
			if (InstigatorController && HitPair.Key && HasAuthority())
			{
				UGameplayStatics::ApplyDamage(HitPair.Key, Damage * HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
			}
		}

		//UGameplayStatics::ApplyDamage(HitCharacter, Damage, InstigatorController, this, UDamageType::StaticClass());
		

		

		

	}
}