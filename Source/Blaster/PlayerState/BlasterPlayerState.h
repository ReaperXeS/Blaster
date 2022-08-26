// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
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

	void AddToDefeats(int32 DefeatsAmount);
	void AddToScore(float ScoreAmountToAdd);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:
	UPROPERTY()
	class ABlasterCharacter* Character = nullptr;

	UPROPERTY()
	class ABlasterPlayerController* Controller = nullptr;

	void UpdateDefeats();
	void UpdateScore();

	UPROPERTY(ReplicatedUsing=OnRep_Defeats)
	int32 Defeats;
};
