// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyPlayerController.h"
#include "Test3/HUD/RemyHUD.h"
#include "Test3/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ARemyPlayerController::BeginPlay() 
{
	Super::BeginPlay();
	RemyHUD = Cast<ARemyHUD>(GetHUD());


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