// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyPlayerController.h"
#include "Test3/HUD/RemyHUD.h"
#include "Test3/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Test3/Character/RemyCharacter.h"

void ARemyPlayerController::BeginPlay() 
{
	Super::BeginPlay();
	RemyHUD = Cast<ARemyHUD>(GetHUD());


}

void ARemyPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	ARemyCharacter* RemyC = Cast<ARemyCharacter>(InPawn);
	if (RemyC) {
		SetHUDHealth(RemyC->GetHealth(), RemyC->GetMaxHealth());
	}
}

void ARemyPlayerController::SetHUDHealth(float Health, float MaxHealth) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->CharacterOverlay && RemyHUD->CharacterOverlay->HealthBar && RemyHUD->CharacterOverlay->HealthText;
	if (bHUDValid) {
		const float HealthPercent = Health / MaxHealth;
		RemyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		RemyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(Health))));
	}
}

void ARemyPlayerController::SetHUDScore(float Score) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->CharacterOverlay && RemyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		RemyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ARemyPlayerController::SetHUDDefeats(int32 Defeats) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid) {
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		RemyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
}

void ARemyPlayerController::SetHUDWeaponAmmo(int32 Ammo, int32 Capacity)
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->WeaponAmmoAmount && RemyHUD->CharacterOverlay->WeaponAmmoBar;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		RemyHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		if (Capacity != 0)
		{
			const float AmmoPercent = (float)Ammo / (float)Capacity;
			RemyHUD->CharacterOverlay->WeaponAmmoBar->SetPercent(AmmoPercent);
		}
	}
}

void ARemyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		RemyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}