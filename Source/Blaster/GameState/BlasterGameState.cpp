// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"

#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"

void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
}

void ABlasterGameState::UpdateTopScore(ABlasterPlayerState* ScoringPlayer)
{
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

void ABlasterGameState::BlueTeamScores()
{
	BlueTeamScore++;

	if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

void ABlasterGameState::RedTeamScores()
{
	RedTeamScore++;
	if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_RedTeamScore()
{
	if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDRedTeamScore(RedTeamScore);
	}
}

void ABlasterGameState::OnRep_BlueTeamScore()
{
	if (ABlasterPlayerController* PlayerController = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
