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
	void SetHUDAnnouncementCountdown(const float CountdownTime);
	void SetHUDGrenades(const int32 Grenades);

	void SetHUDDefeats(int32 Defeats);
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDShield(float Shield, float MaxShield);
	void SetHUDScore(float Score);
	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;
	// Synced with server world clock
	virtual float GetServerTime();

	// Synced with server clock as soon as possible
	virtual void ReceivedPlayer() override;
	void OnMatchStateSet(FName State);
	void HandleMatchHasStarted();
	void HandleCooldown();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	virtual void BeginPlay() override;
	class ABlasterHUD* GetBlasterHUD();
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

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
	void CheckTimeSync(float DeltaSeconds);

	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	// Called when from ServerCheckMatchState();
	UFUNCTION(Client, Reliable)
	void ClientJoinMiddleOfGame(FName StateOfMatch, float Warmup, float Match, float StartingTime, float Cooldown);

public:
private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	/*********************************
	 * Variables set from Game Mode
	 ********************************/
	float CooldownTime = 0.f;
	float LevelStartingTime = 0.f;
	float MatchTime = 0.f;
	float WarmupTime = 0.f;
	uint32 CountDownInt = 0;
	bool bCooldownUrgencyPlaying = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	UFUNCTION()
	void OnRep_MatchState();
};
