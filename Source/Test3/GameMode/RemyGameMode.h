// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "RemyGameMode.generated.h"


namespace MatchState
{
	extern TEST3_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

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

	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	float LevelStartingTime = 0.f;
protected:
	virtual void BeginPlay() override;
	virtual void OnMatchStateSet() override;
private:
	//returns the furthest player start from all players
	AActor* GetFurthestPlayerStart(TArray<AActor*> PlayerStarts, TArray<AActor*> Characters);
	float MinDistanceFromCharacters(AActor* PlayerStart, TArray<AActor*> Characters);
	// Get Closest Player Start After given Distance, if there is non then returns the furthest player start from all players
	AActor* GetClosestPlayerStartAfterDistance(TArray<AActor*> PlayerStarts, TArray<AActor*> Characters);

	float CountDownTime = 0.f;

	UPROPERTY(EditDefaultsOnly)
	float MinSpawnDistance = 5000;

	UPROPERTY(EditDefaultsOnly)
	float MaxSpawnDistance = 10000;


public:
	FORCEINLINE float GetCountDownTime() const { return CountDownTime; }
};
