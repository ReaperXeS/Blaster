// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	void HideDeathMessage();
	void UpdateDeathMessage(FString KilledBy);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetHUDCarriedWeaponType(EWeaponType WeaponType);

	void SetHUDDefeats(int32 Defeats);
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	virtual void OnPossess(APawn* InPawn) override;
protected:
	virtual void BeginPlay() override;
	class ABlasterHUD* GetBlasterHUD();

	void UpdateTextBlockText(class UTextBlock* TextBlock, int32 Value) const;
	void UpdateTextBlockText(UTextBlock* TextBlock, FString Text) const;
public:
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;
};
