// Fill out your copyright notice in the Description page of Project Settings.

#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

void UMenu::OnLevelRemovedFromWorld(ULevel* aLevel, UWorld* aWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(aLevel, aWorld);
}

void UMenu::MenuSetup(int32 aNumbPublicConnections, FString NewMatchType, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumbPublicConnections = aNumbPublicConnections;
	MatchType = NewMatchType;

	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	if (const UWorld* World = GetWorld())
	{
		if (APlayerController* PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	if (const auto GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
	}
}

void UMenu::btnHostClick()
{
	btnHost->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumbPublicConnections, MatchType);
	}
}

void UMenu::btnJoinClick()
{
	btnJoin->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000); // High number because we use Space Wars dev app id for steam
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

bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (btnHost)
	{
		btnHost->OnClicked.AddDynamic(this, &ThisClass::btnHostClick);
	}

	if (btnJoin)
	{
		btnJoin->OnClicked.AddDynamic(this, &ThisClass::btnJoinClick);
	}
	return true;
}

void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (GEngine)
	{
		if (bWasSuccessful)
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Session created successfully going to Lobby: %s"), *PathToLobby));

			UWorld* World = GetWorld();
			if (World)
			{
				World->ServerTravel(PathToLobby);
			}
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString(TEXT("Session creation error")));
			btnHost->SetIsEnabled(true);
		}
	}
}

void UMenu::OnDestroySession(bool bWasSuccessful)
{
}

void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSucessful)
{
	if (bWasSucessful && MultiplayerSessionsSubsystem != nullptr)
	{
		for (FOnlineSessionSearchResult Result : SessionResults)
		{
			FString Id = Result.GetSessionIdStr();
			FString User = Result.Session.OwningUserName;

			FString SettingsValue;
			Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
			if (MatchType == SettingsValue)
			{
				MultiplayerSessionsSubsystem->JoinSession(Result);
				return;
			}
		}
	}

	if (!bWasSucessful || SessionResults.Num() == 0)
	{
		btnJoin->SetIsEnabled(true);
	}
}

void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();

		FString Address;
		if (SessionInterface.IsValid() && SessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
		{
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, TRAVEL_Absolute);
			}
		}
	}

	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		btnJoin->SetIsEnabled(true);
	}
}

void UMenu::OnStartSession(bool bWasSuccessful)
{
}
