// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyPlayerState.h"
#include "Test3/Character/RemyCharacter.h"
#include "Test3/PlayerController/RemyPlayerController.h"


void ARemyPlayerState::AddToScore(float ScoreAmount) {
	Score += ScoreAmount;
	Character = Character == nullptr ? Cast<ARemyCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDScore(Score);
		}
	}
}


void ARemyPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<ARemyCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDScore(Score);
		}
	}
}

