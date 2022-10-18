// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

void ALobbyGameMode::PostLogin(APlayerController* aNewPlayer)
{
	Super::PostLogin(aNewPlayer);

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* SessionSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		check(SessionSubsystem);

		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Number of Players: %d"), GameState.Get()->PlayerArray.Num()));
		if (NumberOfPlayer == SessionSubsystem->DesiredNumPublicConnections)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;

				const FString MatchType = SessionSubsystem->DesiredMatchType;
				FString MapAddress = "";
				if (MatchType == "FreeForAll")
				{
					MapAddress = FString("/Game/Maps/BlasterMap?listen");
				}
				else if (MatchType == "TeamDeathMatch")
				{
					MapAddress = FString("/Game/Maps/TeamDeathMatch?listen");
				}
				else if (MatchType == "CaptureTheFlag")
				{
					MapAddress = FString("/Game/Maps/CaptureTheFlag?listen");
				}

				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Travelling to %s"), *MapAddress));
				if (!World->ServerTravel(MapAddress))
				{
					GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Travelling to %s FAILED!!!"), *MapAddress));
				}
			}
		}
	}
}
