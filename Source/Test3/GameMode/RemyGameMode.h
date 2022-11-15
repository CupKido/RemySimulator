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
	ARemyGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void PlayerEliminated(class ARemyCharacter* ElimmedCharacter, class ARemyPlayerController* VictimController, ARemyPlayerController* AttackerController);
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);

	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	AActor* GetFurthestPlayerStart(TArray<AActor*> PlayerStarts, TArray<AActor*> Characters);
	float MinDistanceFromCharacters(AActor* PlayerStart, TArray<AActor*> Characters);

	float CountDownTime = 0.f;
};
