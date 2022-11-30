// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/Character.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	RocketMesh->SetupAttachment(RootComponent);
	RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();
	if (TrailSystem) {
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TrailSystem, GetRootComponent(), FName(), GetActorLocation(), GetActorRotation(), EAttachLocation::KeepWorldPosition, false);
	}
	if (!HasAuthority()) {
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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
				5.f, // minimum damage
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


	}

	GetWorldTimerManager().SetTimer(
		DestroyTimer, this, &AProjectileRocket::DestroyTimerFinished, DestroyTime
	);

	if (ImpactParticles) {
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound) {
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (RocketMesh)
	{
		RocketMesh->SetVisibility(false);
	}
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}


void AProjectileRocket::DestroyTimerFinished()
{
	Destroy();
}

void AProjectileRocket::Destroyed()
{
}