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
	void SetHUDMatchCountdown(const float CountdownTime);

	void SetHUDDefeats(int32 Defeats);
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	virtual void OnPossess(APawn* InPawn) override;
	void CheckTimeSync(float DeltaSeconds);
	virtual void Tick(float DeltaSeconds) override;
	// Synced with server world clock
	virtual float GetServerTime();

	// Synced with server clock as soon as possible
	virtual void ReceivedPlayer() override;
protected:
	virtual void BeginPlay() override;
	class ABlasterHUD* GetBlasterHUD();

	void UpdateTextBlockText(class UTextBlock* TextBlock, int32 Value) const;
	void UpdateTextBlockText(UTextBlock* TextBlock, FString Text) const;
	void SetHUDTime();

	/***************************************/
	/* Sync Time between client and Server */
	/***************************************/

	// Request the current server time, passing the client time when the request sent
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(const float TimeOfClientRequest);

	// Reports the current server time to the client in response to ServerRequestServerTime
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest);

	// Difference between client and server time
	float ClientServerDelta = 0;

	// Sync with server every (x) Seconds
	UPROPERTY(EditAnywhere, Category = "Time")
	float TimeSyncFrequency = 5.f;

	float TimeSyncRunningTime = 0.f;
public:
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	float MatchTime = 120.f;
	uint32 CountDownInt = 0;
};
