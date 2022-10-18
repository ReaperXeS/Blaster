// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CTFGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ACTFGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	virtual void PlayerEliminated(class ABlasterCharacter* EliminatedCharacter, class ABlasterPlayerController* EliminatedController, ABlasterPlayerController* AttackerController) override;
	void FlagCaptured(const class AFlag* Flag, const class AFlagZone* Zone) const;
};
