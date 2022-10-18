// Fill out your copyright notice in the Description page of Project Settings.


#include "CTFGameMode.h"

#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/Weapon/Flag.h"

void ACTFGameMode::PlayerEliminated(ABlasterCharacter* EliminatedCharacter, ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController)
{
	ABlasterGameMode::PlayerEliminated(EliminatedCharacter, EliminatedController, AttackerController);
}

void ACTFGameMode::FlagCaptured(const AFlag* Flag, const AFlagZone* Zone) const
{
	if (Flag && Zone && Flag->GetTeam() == Zone->Team)
	{
		if (ABlasterGameState* BGameState = GetGameState<ABlasterGameState>())
		{
			if (Zone->Team == ETeam::ET_Blue)
			{
				BGameState->BlueTeamScores();
			}
			else if (Zone->Team == ETeam::ET_Red)
			{
				BGameState->RedTeamScores();
			}
		}
	}
}
