// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::AddToScore(float ScoreAmountToAdd)
{
	SetScore(GetScore() + ScoreAmountToAdd);
	UpdateScore();
}

void ABlasterPlayerState::UpdateDeathMessage(FString KillerName)
{
	KilledBy = KillerName;
	UpdateDeathMessageHUD();
}

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	DOREPLIFETIME(ABlasterPlayerState, KilledBy);
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

void ABlasterPlayerState::UpdateDefeats()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::UpdateDeathMessageHUD()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->UpdateDeathMessage(KilledBy);
		}
	}
}

void ABlasterPlayerState::UpdateScore()
{
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::OnRep_Team()
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BlasterCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::SetTeam(const ETeam NewTeam)
{
	Team = NewTeam;
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BlasterCharacter->SetTeamColor(Team);
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	UpdateScore();
}

void ABlasterPlayerState::OnRep_Defeats()
{
	UpdateDefeats();
}

void ABlasterPlayerState::OnRep_KilledBy()
{
	UpdateDeathMessageHUD();
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	UpdateDefeats();
}
