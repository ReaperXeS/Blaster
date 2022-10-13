// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/Team.h"
#include "GameFramework/PlayerState.h"
#include "BlasterPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	virtual void OnRep_Score() override;

	UFUNCTION()
	void OnRep_Defeats();

	UFUNCTION()
	void OnRep_KilledBy();

	void AddToDefeats(int32 DefeatsAmount);
	void AddToScore(float ScoreAmountToAdd);
	void UpdateDeathMessage(FString KillerName);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY()
	class ABlasterCharacter* Character = nullptr;

	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;

	void UpdateDefeats();
	void UpdateDeathMessageHUD();
	void UpdateScore();

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;

	UPROPERTY(ReplicatedUsing=OnRep_KilledBy)
	FString KilledBy;

	UPROPERTY(Replicated)
	ETeam Team = ETeam::ET_NoTeam;
public:
	FORCEINLINE ETeam GetTeam() const { return Team; }
	FORCEINLINE void SetTeam(const ETeam NewTeam) { Team = NewTeam; }
};
