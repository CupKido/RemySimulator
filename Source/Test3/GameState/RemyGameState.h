// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "RemyGameState.generated.h"

/**
 * 
 */
UCLASS()
class TEST3_API ARemyGameState : public AGameState
{
	GENERATED_BODY()
	

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void UpdateTopScore(class ARemyPlayerState* ScoringPlayer);

	UPROPERTY(Replicated)
	TArray<ARemyPlayerState*> TopScoringPlayers;

	float TopScore = 0.f;
protected:

private:
};
