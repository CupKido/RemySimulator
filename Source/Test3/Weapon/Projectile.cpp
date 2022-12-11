// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Test3/Character/RemyCharacter.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Test3/Test3.h"

// Sets default values
AProjectile::AProjectile()
{

	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);

	
}


void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	if (Tracer) {
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority()) {
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
	if (bDestroyProjectileAfterTime) {
		GetWorldTimerManager().SetTimer(
			ProjectileDestroyTimer, this, &AProjectile::ProjectileDestroyTimerFinished, ProjectileDestroyTime
		);
	}
}

void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	
	Destroy();
}

void AProjectile::ProjectileDestroyTimerFinished()
{
	OnHit(nullptr, nullptr, nullptr, FVector::ZeroVector, FHitResult());
}



void AProjectile::SpawnTrailSystem()
{
	if (TrailSystem) {
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
}

void AProjectile::ExplodeDamage(AActor* OtherActor)
{
	if (OtherActor == GetOwner()) {
		return;
	}
	APawn* FiringPawn = GetInstigator();
	if (FiringPawn && HasAuthority()) {
		AController* FiringController = FiringPawn->GetController();
		if (FiringController) {
			TArray<AActor*> IgnoredActors;
			IgnoredActors.Add(FiringPawn);
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // World context object
				Damage, // Base Damage
				Damage * 0.2f, // minimum damage
				GetActorLocation(), // Origin Location
				InnerRadius, // Inner radius where players receive full damage
				OuterRadius, // Outer radius until where player receive fading damage
				1.5f, // Damage fall off
				UDamageType::StaticClass(), // damage type
				IgnoredActors
				/*TArray<AActor*>()*/, //ignored actors
				this, // Damage causer
				FiringController // Instigator Controller
			);

			LaunchPlayers();
			
		}


	}
}

void AProjectile::LaunchPlayers()
{
	TArray<AActor*> Characters;
	UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Characters);
	for (AActor* Character : Characters) {
		ACharacter* ch = Cast<ACharacter>(Character);
		if (ch) {
			FVector VelocityVector = (ch->GetActorLocation() - GetActorLocation());
			float size = VelocityVector.Size();
			/*if (GEngine) {
				FString temp = FString::Printf(TEXT("%f"), size);
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, temp);
			}*/
			if (size < LaunchOuterRadius) {
				VelocityVector.Normalize();
				FHitResult FireHit;
				UWorld* World = GetWorld();
				if (World) {
					FVector Start = GetActorLocation() + (VelocityVector);
					FVector End = GetActorLocation() + ((ch->GetActorLocation() - GetActorLocation()) - (VelocityVector));
					World->LineTraceSingleByChannel(
						FireHit, Start, End, ECollisionChannel::ECC_Camera);

					if (!FireHit.bBlockingHit || FireHit.ImpactPoint == ch->GetActorLocation())
					{
						if (size < LaunchInnerRadius) {
							VelocityVector = (LaunchStrength * 100) * VelocityVector;
						}
						else {
							VelocityVector = (LaunchStrength * 10000 / size) * VelocityVector;
						}

						ch->LaunchCharacter(VelocityVector, false, false);
					}
				}

				/*VelocityVector = (LaunchStrength * 10000 / size) * VelocityVector;
				ch->LaunchCharacter(VelocityVector, false, false);*/
			}
		}
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AProjectile::StartDestroyTimer()
{
	GetWorldTimerManager().SetTimer(
		DestroyTimer, this, &AProjectile::DestroyTimerFinished, DestroyTime
	);
}

void AProjectile::DestroyTimerFinished()
{
	Destroy();
}

void AProjectile::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

