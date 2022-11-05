// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyGameMode.h"
#include "Test3/PlayerController/RemyPlayerController.h"
#include "Test3/Character/RemyCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Test3/PlayerState/RemyPlayerState.h"

void ARemyGameMode::PlayerEliminated(class ARemyCharacter* ElimmedCharacter, class ARemyPlayerController* VictimController, ARemyPlayerController* AttackerController) {

	ARemyPlayerState* AttackerPlayerState = AttackerController ? Cast<ARemyPlayerState>(AttackerController->PlayerState) : nullptr;
	ARemyPlayerState* VictimPlayerState = VictimController ? Cast<ARemyPlayerState>(VictimController->PlayerState) : nullptr;
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
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