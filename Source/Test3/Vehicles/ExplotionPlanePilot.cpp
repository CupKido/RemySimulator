// Fill out your copyright notice in the Description page of Project Settings.


#include "ExplotionPlanePilot.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

void AExplotionPlanePilot::BeginPlay() {
	Super::BeginPlay();
	if (HasAuthority()) {
		Fuselage->OnComponentHit.AddDynamic(this, &AExplotionPlanePilot::OnHit);
	}
}

void AExplotionPlanePilot::Tick(float DeltaTime) {
	Super::Tick(DeltaTime);
}

void AExplotionPlanePilot::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) {
	ExplodeDamage();
	GetController()->Possess(GetInstigator());
	Destroy();
}

void AExplotionPlanePilot::ExplodeDamage() {

	APawn* FiringPawn = GetInstigator();
	if (this && HasAuthority()) {
		if (GetController()) {
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				GetInstigator(), // World context object
				Damage, // Base Damage
				Damage * 0.2f, // minimum damage
				GetActorLocation(), // Origin Location
				InnerRadius, // Inner radius where players receive full damage
				OuterRadius, // Outer radius until where player receive fading damage
				1.5f, // Damage fall off
				UDamageType::StaticClass(), // damage type
				/*IgnoredActors*/
				TArray<AActor*>(), //ignored actors
				this, // Damage causer
				GetController() // Instigator Controller
			);

		}


	}
}

void AExplotionPlanePilot::Destroyed()
{
	Super::Destroyed();
	if (ImpactParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	//APawn* pawn = Cast<APawn>(GetOwner());
	/*if (pawn) {*/

	/*}*/
}

void AExplotionPlanePilot::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("SelfExplode", IE_Pressed, this, &AExplotionPlanePilot::SelfExplode);
}

void AExplotionPlanePilot::SelfExplode() {
	ExplodeDamage();
	GetController()->Possess(GetInstigator());
	Destroy();
}

