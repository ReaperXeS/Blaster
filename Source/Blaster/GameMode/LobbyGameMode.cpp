// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::PostLogin(APlayerController* aNewPlayer)
{
	Super::PostLogin(aNewPlayer);

	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();
	GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Number of Players: %d"), GameState.Get()->PlayerArray.Num()));
	if (NumberOfPlayer == 2)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			bUseSeamlessTravel = true;
			FString MapAddress = FString("/Game/Maps/BlasterMap?listen");
			GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Yellow, FString::Printf(TEXT("Travelling to %s"), *MapAddress));
			if (!World->ServerTravel(MapAddress))
			{
				GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Red, FString::Printf(TEXT("Travelling to %s FAILED!!!"), *MapAddress));
			}
		}
	}
}
