// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RemyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class TEST3_API ARemyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class ARemyCharacter* ElimmedCharacter, class ARemyPlayerController* VictimController, ARemyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

private:

	AActor* GetFurthestPlayerStart(TArray<AActor*> PlayerStarts, TArray<AActor*> Characters);
	float MinDistanceFromCharacters(AActor* PlayerStart, TArray<AActor*> Characters);
};
