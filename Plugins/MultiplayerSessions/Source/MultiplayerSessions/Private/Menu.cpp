// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"	
#include "Components/EditableText.h"
#include "Components/CheckBox.h"


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Internationalization/Text.h"
#include "OnlineSubsystem.h"

void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	useDebug = true;
	
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}
	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UMenu::EnableButtons() {
	JoinButton->SetIsEnabled(true);
	HostButton->SetIsEnabled(true);
}

void UMenu::DisableButtons() {
	HostButton->SetIsEnabled(false);
	JoinButton->SetIsEnabled(false);
}

void UMenu::DebugPrint(float duration, FColor Color, FString Text) {
	if (useDebug) {
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				60.f,
				Color,
				Text
			);
		}
	}
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (MultiplayerSessionsSubsystem->GetSessionKey() != FString("")) {
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.f,
					FColor::Green,
					MultiplayerSessionsSubsystem->GetSessionKey()
				);
			}
		}
		else {
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					60.f,
					FColor::Green,
					FString(TEXT("Session is public"))
				);
			}
		}
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
		
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		EnableButtons();
	}
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}
	bool WrongKey = false;
	if (bWasSuccessful) {
		DebugPrint(15.f, FColor::Red, FString("Find seesion was successful"));
	}
	for (auto Result : SessionResults)
	{
		FString SettingsValue;
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		FString KeyValue;
		Result.Session.SessionSettings.Get(FName("SessionKey"), KeyValue);
		if (SettingsValue == MatchType)
		{
			DebugPrint(15.f, FColor::Blue, Result.GetSessionIdStr());
			DebugPrint(15.f, FColor::Blue, FString("Session Key:"));
			DebugPrint(15.f, FColor::Blue, KeyValue);
			DebugPrint(15.f, FColor::Blue, FString("Inserted Key:"));
			DebugPrint(15.f, FColor::Blue, KeyBox->GetText().ToString());
			if (KeyBox->GetText().ToString() == KeyValue) {
				DebugPrint(15.f, FColor::Blue, FString("Joining!"));
				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
			else {
				WrongKey = true;
			}
			
		}
	}
	if (!bWasSuccessful) {
		DebugPrint(15.f, FColor::Red, FString("Find seesion was unsuccessful"));
	}
	if (SessionResults.Num() == 0)
	{
		DebugPrint(15.f, FColor::Red, FString("No sessions availible"));
	}
	if (WrongKey) {
		DebugPrint(15.f, FColor::Red, FString("Inserted key did not match any session key!"));
	}
	EnableButtons();
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	DebugPrint(15.f, FColor::Blue, FString("Joining:"));
	if (Subsystem)
	{
		DebugPrint(15.f, FColor::Blue, FString("Subsystem is valid"));
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			DebugPrint(15.f, FColor::Blue, FString("Session interface is valid"));
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			DebugPrint(15.f, FColor::Blue, FString::Printf(TEXT("Session Address: %s"), *Address));
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				DebugPrint(15.f, FColor::Blue, FString("Travelling..."));
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::HostButtonClicked()
{
	DisableButtons();
	if (MultiplayerSessionsSubsystem)
	{
		FString trueS = "True";
		FString falseS = "False";
		
		MultiplayerSessionsSubsystem->SetCreateKey(PrivateCheckBox->IsChecked());
		DebugPrint(15.f, FColor::Blue, FString::Printf(TEXT("Session is Private: %s"), (PrivateCheckBox->IsChecked() ? *trueS : *falseS)));
		MultiplayerSessionsSubsystem->SetLan(LanCheckBox->IsChecked());
		DebugPrint(15.f, FColor::Blue, FString::Printf(TEXT("Session is Lan: %s"), (LanCheckBox->IsChecked() ? *trueS : *falseS)));
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

void UMenu::JoinButtonClicked()
{
	DisableButtons();
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->SetLan(LanCheckBox->IsChecked());
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}

void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}
