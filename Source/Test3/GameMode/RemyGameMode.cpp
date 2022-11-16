// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyGameMode.h"
#include "Test3/PlayerController/RemyPlayerController.h"
#include "Test3/Character/RemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Test3/PlayerState/RemyPlayerState.h"
#include "Test3/PlayerController/RemyPlayerController.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ARemyGameMode::ARemyGameMode() {
	bDelayedStart = true;
}

void ARemyGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

void ARemyGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart) {
		CountDownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountDownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountDownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountDownTime <= 0.f) {
			RestartGame();
		}
	}
}

void ARemyGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) 
	{
		ARemyPlayerController* RemyPlayer = Cast<ARemyPlayerController>(*It);
		if (RemyPlayer) {
			RemyPlayer->OnMatchStateSet(MatchState);
		}
	}
}

void ARemyGameMode::PlayerEliminated(class ARemyCharacter* ElimmedCharacter, class ARemyPlayerController* VictimController, ARemyPlayerController* AttackerController) {

	ARemyPlayerState* AttackerPlayerState = AttackerController ? Cast<ARemyPlayerState>(AttackerController->PlayerState) : nullptr;
	ARemyPlayerState* VictimPlayerState = VictimController ? Cast<ARemyPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);	
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	if(ElimmedCharacter){
		ElimmedCharacter->Elim();
	}
}

void ARemyGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController) 
{
	if (ElimmedCharacter) {
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController) {
		TArray<AActor*> PlayerStarts;
		TArray<AActor*> Characters;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		UGameplayStatics::GetAllActorsOfClass(this, ACharacter::StaticClass(), Characters);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, GetFurthestPlayerStart(PlayerStarts, Characters));
	}
}

AActor* ARemyGameMode::GetFurthestPlayerStart(TArray<AActor*> PlayerStarts, TArray<AActor*> Characters)
{
	int32 Max = 0;
	int32 Distance;
	AActor* MaxPlayerStart = nullptr;
	for (int i = 0; i < PlayerStarts.Num(); i++) {
		Distance = MinDistanceFromCharacters(PlayerStarts[i], Characters);
		if (Max < Distance) {
			Max = Distance;
			MaxPlayerStart = PlayerStarts[i];
		}
	}
	if (MaxPlayerStart) {
		return MaxPlayerStart;
	}
	return nullptr;
}

float ARemyGameMode::MinDistanceFromCharacters(AActor* PlayerStart, TArray<AActor*> Characters) {
	int32 Min = 2147483000;
	int32 Distance;
	for (int i = 0; i < Characters.Num(); i++ ) {
		Distance = (PlayerStart->GetActorLocation() - Characters[i]->GetActorLocation()).Size();
		if (Distance < Min) Min = Distance;
	}
	return Min;
}