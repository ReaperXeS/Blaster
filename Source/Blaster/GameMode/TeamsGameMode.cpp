// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"

void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Sort player team
	ABlasterPlayerState* BPlayerState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
	if (ABlasterGameState* const BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)); BGameState && BPlayerState)
	{
		if (BPlayerState->GetTeam() == ETeam::ET_NoTeam)
		{
			if (BGameState->BlueTeam.Num() > BGameState->RedTeam.Num())
			{
				BPlayerState->SetTeam(ETeam::ET_Red);
				BGameState->RedTeam.AddUnique(BPlayerState);
			}
			else
			{
				BPlayerState->SetTeam(ETeam::ET_Blue);
				BGameState->BlueTeam.AddUnique(BPlayerState);
			}
		}
	}
}

void ATeamsGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	// Remove player from team
	ABlasterPlayerState* BPlayerState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (ABlasterGameState* const BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)); BGameState && BPlayerState)
	{
		if (BGameState->RedTeam.Contains(BPlayerState))
		{
			BGameState->RedTeam.Remove(BPlayerState);
		}

		if (BGameState->BlueTeam.Contains(BPlayerState))
		{
			BGameState->BlueTeam.Remove(BPlayerState);
		}
	}
}

void ATeamsGameMode::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();

	// Sort players teams
	if (ABlasterGameState* const BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
	{
		for (auto PlayerState : BGameState->PlayerArray)
		{
			if (ABlasterPlayerState* BPlayerState = Cast<ABlasterPlayerState>(PlayerState.Get()))
			{
				if (BPlayerState->GetTeam() == ETeam::ET_NoTeam)
				{
					if (BGameState->BlueTeam.Num() > BGameState->RedTeam.Num())
					{
						BPlayerState->SetTeam(ETeam::ET_Red);
						BGameState->RedTeam.AddUnique(BPlayerState);
					}
					else
					{
						BPlayerState->SetTeam(ETeam::ET_Blue);
						BGameState->BlueTeam.AddUnique(BPlayerState);
					}
				}
			}
		}
	}
}
