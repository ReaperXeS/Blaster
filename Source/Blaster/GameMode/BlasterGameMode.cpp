// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"

namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? AttackerController->GetPlayerState<ABlasterPlayerState>() : nullptr;
	ABlasterPlayerState* EliminatedPlayerState = EliminatedController ? EliminatedController->GetPlayerState<ABlasterPlayerState>() : nullptr;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	if (AttackerPlayerState && AttackerPlayerState != EliminatedPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> Leaders;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			Leaders.Add(LeadPlayer);
		}
		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		// If the attacker is now the leader, broadcast the event 
		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			if (ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn()))
			{
				Leader->MulticastGainedTheLead();
			}
		}

		// Lost the lead players
		for (int32 i = 0; i < Leaders.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(Leaders[i]))
			{
				if (ABlasterCharacter* Loser = Cast<ABlasterCharacter>(Leaders[i]->GetPawn()))
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}

	if (EliminatedPlayerState)
	{
		EliminatedPlayerState->AddToDefeats(1);

		if (AttackerPlayerState)
		{
			EliminatedPlayerState->UpdateDeathMessage(AttackerPlayerState->GetPlayerName());
		}
	}

	if (EliminatedCharacter)
	{
		EliminatedCharacter->EliminationServer(false);
	}

	if (AttackerPlayerState && EliminatedPlayerState)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ABlasterPlayerController* BlasterPlayerController = Cast<ABlasterPlayerController>(*It))
			{
				BlasterPlayerController->BroadcastEliminationMessage(AttackerPlayerState, EliminatedPlayerState);
			}
		}
	}
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedCharacter, AController* EliminatedController)
{
	if (EliminatedCharacter)
	{
		EliminatedCharacter->Reset();
		EliminatedCharacter->Destroy();
	}

	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);

		const int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[Selection]);
	}
}

void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr)
	{
		return;
	}

	if (ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>(); BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}

	if (ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn()))
	{
		CharacterLeaving->EliminationServer(true);
	}
}

void ABlasterGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;

		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float Damage)
{
	return Damage;
}

void ABlasterGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It))
		{
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamMatch);
		}
	}
}
