// Fill out your copyright notice in the Description page of Project Settings.


#include "RemyPlayerController.h"
#include "Test3/HUD/RemyHUD.h"
#include "Test3/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Test3/Character/RemyCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Test3/GameMode/RemyGameMode.h"
#include "Test3/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Test3/RemyComponent/CombatComponent.h"
#include "Test3/GameState/RemyGameState.h"
#include "Test3/PlayerState/RemyPlayerState.h"


void ARemyPlayerController::BeginPlay() 
{
	Super::BeginPlay();
	RemyHUD = Cast<ARemyHUD>(GetHUD());
	ServerCheckMatchState();
}
void ARemyPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ARemyPlayerController, MatchState);
}



void ARemyPlayerController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);

	ARemyCharacter* RemyC = Cast<ARemyCharacter>(InPawn);
	if (RemyC) {
		SetHUDHealth(RemyC->GetHealth(), RemyC->GetMaxHealth());
	}
}

void ARemyPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetHUDTime();
	CheckTimeSync(DeltaTime);
	PollInit();
}

void ARemyPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if (IsLocalController() && TimeSyncFrequency <= TimeSyncRunningTime) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ARemyPlayerController::ServerCheckMatchState_Implementation()
{
	ARemyGameMode* GameMode = Cast<ARemyGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

void ARemyPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;
	OnMatchStateSet(MatchState);
	if (RemyHUD && MatchState == MatchState::WaitingToStart)
	{
		RemyHUD->AddAnnouncement();
	}
}

void ARemyPlayerController::SetHUDHealth(float Health, float MaxHealth) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->CharacterOverlay && RemyHUD->CharacterOverlay->HealthBar && RemyHUD->CharacterOverlay->HealthText;
	if (bHUDValid) {
		const float HealthPercent = Health / MaxHealth;
		RemyHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		RemyHUD->CharacterOverlay->HealthText->SetText(FText::FromString(FString::Printf(TEXT("%d"), FMath::CeilToInt(Health))));
	}
	else
	{
		bInitializeCharacterOverlay = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

void ARemyPlayerController::SetHUDScore(float Score) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->CharacterOverlay && RemyHUD->CharacterOverlay->ScoreAmount;
	if (bHUDValid) {
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		RemyHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDScore = Score;
	}
}

void ARemyPlayerController::SetHUDDefeats(int32 Defeats) {
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid) {
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		RemyHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else {
		bInitializeCharacterOverlay = true;
		HUDDefeats = Defeats;
	}
}

void ARemyPlayerController::SetHUDWeaponAmmo(int32 Ammo, int32 Capacity)
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->WeaponAmmoAmount && RemyHUD->CharacterOverlay->WeaponAmmoBar;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		RemyHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
		if (Capacity != 0)
		{
			const float AmmoPercent = (float)Ammo / (float)Capacity;
			RemyHUD->CharacterOverlay->WeaponAmmoBar->SetPercent(AmmoPercent);
		}
	}
}

void ARemyPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD &&
		RemyHUD->CharacterOverlay &&
		RemyHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid) {
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		RemyHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ARemyPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->CharacterOverlay && RemyHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid) {

		if (CountdownTime < 0.f)
		{
			RemyHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = FMath::FloorToInt(CountdownTime) % 60;
		

		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		RemyHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

void ARemyPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{

	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	bool bHUDValid = RemyHUD && RemyHUD->Announcement && RemyHUD->Announcement->WarmupTime;
	if (bHUDValid) {
		if (CountdownTime < 0.f)
		{
			RemyHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
		int32 Seconds = FMath::FloorToInt(CountdownTime) % 60;


		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		RemyHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

void ARemyPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown)  TimeLeft = WarmupTime + MatchTime + CooldownTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);

	if (HasAuthority()) {
		RemyGameMode = RemyGameMode == nullptr ? Cast<ARemyGameMode>(UGameplayStatics::GetGameMode(this)) : RemyGameMode;
		if (RemyGameMode) {
			SecondsLeft = FMath::CeilToInt(RemyGameMode->GetCountDownTime() + LevelStartingTime);
		}
	}


	if (CountdownInt != SecondsLeft) {
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown) {
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress) {
			SetHUDMatchCountdown(TimeLeft);
		}
	}
	CountdownInt = SecondsLeft;
}

void ARemyPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr) {
		if (RemyHUD && RemyHUD->CharacterOverlay)
		{
			CharacterOverlay = RemyHUD->CharacterOverlay;
			if (CharacterOverlay) {
				SetHUDHealth(HUDHealth, HUDMaxHealth);
				SetHUDScore(HUDScore);
				SetHUDDefeats(HUDDefeats);
			}
		}
	}
}



void ARemyPlayerController::ServerRequestServerTime_Implementation(float TimeOfclientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfclientRequest, ServerTimeOfReceipt);
}

void ARemyPlayerController::ClientReportServerTime_Implementation(float TimeOfclientRequest, float TimeServerReceivedClientRequest)
{
	float RoundTrip = GetWorld()->GetTimeSeconds() - TimeOfclientRequest;
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTrip * 0.5f);
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ARemyPlayerController::GetServerTime()
{
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ARemyPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController()) {
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ARemyPlayerController::OnMatchStateSet(FName State)
{
	MatchState = State;
	
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if(MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}


void ARemyPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ARemyPlayerController::HandleMatchHasStarted()
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	if (RemyHUD) {
		RemyHUD->AddCharacterOverlay();
		if (RemyHUD->Announcement) {
			RemyHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ARemyPlayerController::HandleCooldown()
{
	RemyHUD = RemyHUD == nullptr ? Cast<ARemyHUD>(GetHUD()) : RemyHUD;
	if (RemyHUD) {
		RemyHUD->CharacterOverlay->RemoveFromParent();
		bool bHUDValid = RemyHUD->Announcement && 
			RemyHUD->Announcement->AnnouncementText && 
			RemyHUD->Announcement->InfoText;
		if (bHUDValid) {
			RemyHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText("New match starts in:");
			RemyHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			ARemyGameState* RemyGameState = Cast<ARemyGameState>(UGameplayStatics::GetGameState(this));
			ARemyPlayerState* RemyPlayerState = GetPlayerState<ARemyPlayerState>();
			if (RemyGameState && RemyPlayerState) {
				TArray<ARemyPlayerState*> TopPlayers = RemyGameState->TopScoringPlayers;
				FString InfoTextString;
				if (TopPlayers.Num() == 0) {
					InfoTextString = FString("There is no champion.");
				}
				else
				{
					if (TopPlayers.Num() == 1) {
						if (TopPlayers[0] == RemyPlayerState) {
							InfoTextString = FString("You are the champion!");
						}
						else {
							InfoTextString = FString::Printf(TEXT("%s is the champion!"), *TopPlayers[0]->GetPlayerName());
						}
					}
					else if (TopPlayers.Num() > 1) {
						InfoTextString = FString("The champion are:");
						for (auto TiedPlayer : TopPlayers) {
							InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
						}
					}
					InfoTextString.Append(FString::Printf(TEXT("\nwith the score of: %d!\n"), TopPlayers[0]->GetScore()));
				}
				
				RemyHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
			
		}
	}
	ARemyCharacter* RemyC = Cast<ARemyCharacter>(GetPawn());
	if (RemyC && RemyC->GetCombat()) {
		RemyC->bDisableGameplay = true;
		RemyC->GetCombat()->FireButtonPressed(false);
	}
}