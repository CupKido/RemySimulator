// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "RemyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class TEST3_API ARemyPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void OnRep_Score() override;
	
	void AddToScore(float ScoreAmount);
private:
	class ARemyCharacter* Character;
	class ARemyPlayerController* Controller;
};
