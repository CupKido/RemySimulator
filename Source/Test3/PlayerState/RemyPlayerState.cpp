// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyPlayerState.h"
#include "Test3/Character/RemyCharacter.h"
#include "Test3/PlayerController/RemyPlayerController.h"
#include "Net/UnrealNetwork.h"

void ARemyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARemyPlayerState, Defeats);
}

void ARemyPlayerState::AddToScore(float ScoreAmount) {
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<ARemyCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDScore(GetScore());
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
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ARemyPlayerState::AddToDefeats(int32 DefeatsAmount) {
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<ARemyCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ARemyPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<ARemyCharacter>(GetPawn()) : Character;
	if (Character) {
		Controller = Controller == nullptr ? Cast<ARemyPlayerController>(Character->Controller) : Controller;
		if (Controller) {
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

