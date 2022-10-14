// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/HUD/Announcement.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Components/Image.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/HUD/ReturnToMainMenu.h"

void ABlasterPlayerController::UpdateDeathMessage(const FString KilledBy)
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay &&
		GetBlasterHUD()->CharacterOverlay->DeathMessage &&
		GetBlasterHUD()->CharacterOverlay->KilledBy)
	{
		GetBlasterHUD()->CharacterOverlay->KilledBy->SetText(FText::FromString(KilledBy));
		GetBlasterHUD()->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Visible);
		GetBlasterHUD()->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Visible);
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(const int32 Ammo)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->WeaponAmmoAmount, Ammo);
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(const int32 Ammo)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->CarriedAmmoAmount, Ammo);
	}
}

void ABlasterPlayerController::SetHUDCarriedWeaponType(const EWeaponType WeaponType)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		FString WeaponTypeString;
		switch (WeaponType)
		{
		case EWeaponType::EWT_AssaultRifle:
			WeaponTypeString = "Assault Rifle";
			break;
		case EWeaponType::EWT_RocketLauncher:
			WeaponTypeString = "Rocket Launcher";
			break;
		case EWeaponType::EWT_Pistol:
			WeaponTypeString = "Pistol";
			break;
		case EWeaponType::EWT_Shotgun:
			WeaponTypeString = "Shotgun";
			break;
		case EWeaponType::EWT_SubMachineGun:
			WeaponTypeString = "Sub Machine Gun";
			break;
		case EWeaponType::EWT_SniperRifle:
			WeaponTypeString = "Sniper Rifle";
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			WeaponTypeString = "Grenade Launcher";
			break;
		default:
			WeaponTypeString = "Patate";
			break;
		}
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->CarriedAmmoWeaponType, WeaponTypeString);
	}
}

void ABlasterPlayerController::SetHUDMatchCountdown(const float CountdownTime)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		if (CountdownTime <= 0.f)
		{
			UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->MatchCountdownText, FString());
		}
		else
		{
			const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
			const int32 Seconds = CountdownTime - Minutes * 60;
			const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
			UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->MatchCountdownText, CountdownText);
		}
	}
}

void ABlasterPlayerController::SetHUDAnnouncementCountdown(const float CountdownTime)
{
	if (GetBlasterHUD() && GetBlasterHUD()->Announcement)
	{
		if (CountdownTime <= 0.f)
		{
			UpdateTextBlockText(GetBlasterHUD()->Announcement->WarmupTime, FString());
		}
		else
		{
			const int32 Minutes = FMath::FloorToInt(CountdownTime / 60);
			const int32 Seconds = CountdownTime - Minutes * 60;
			const FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
			UpdateTextBlockText(GetBlasterHUD()->Announcement->WarmupTime, CountdownText);
		}
	}
}

void ABlasterPlayerController::SetHUDGrenades(const int32 Grenades)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->GrenadeText, Grenades);
	}
}

void ABlasterPlayerController::SetHUDDefeats(const int32 Defeats)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->DefeatsAmount, Defeats);
	}
}

void ABlasterPlayerController::SetHUDHealth(const float Health, const float MaxHealth)
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay &&
		GetBlasterHUD()->CharacterOverlay->HealthBar &&
		GetBlasterHUD()->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		GetBlasterHUD()->CharacterOverlay->HealthBar->SetPercent(HealthPercent);

		const FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		GetBlasterHUD()->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::SetHUDShield(const float Shield, const float MaxShield)
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay &&
		GetBlasterHUD()->CharacterOverlay->ShieldBar &&
		GetBlasterHUD()->CharacterOverlay->ShieldText)
	{
		const float ShieldPercent = Shield / MaxShield;
		GetBlasterHUD()->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);

		const FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		GetBlasterHUD()->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
}

void ABlasterPlayerController::SetHUDScore(const float Score)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->ScoreAmount, FMath::FloorToInt(Score));
	}
}

void ABlasterPlayerController::BroadcastEliminationMessage(const APlayerState* Attacker, const APlayerState* Victim)
{
	ClientEliminationMessage(Attacker, Victim);
}

void ABlasterPlayerController::ClientEliminationMessage_Implementation(const APlayerState* Attacker, const APlayerState* Victim)
{
	if (const APlayerState* LocalPlayerState = GetPlayerState<APlayerState>(); Attacker && Victim && LocalPlayerState && GetBlasterHUD())
	{
		if (Attacker == LocalPlayerState && Attacker != Victim)
		{
			GetBlasterHUD()->AddEliminationAnnouncement("You", Victim->GetPlayerName());
			return;
		}

		if (Victim == LocalPlayerState && Attacker != LocalPlayerState)
		{
			GetBlasterHUD()->AddEliminationAnnouncement(Attacker->GetPlayerName(), "You");
			return;
		}

		if (Attacker == Victim && Attacker == LocalPlayerState)
		{
			GetBlasterHUD()->AddEliminationAnnouncement("You", "Yourself");
			return;
		}

		if (Attacker == Victim && Attacker != LocalPlayerState)
		{
			GetBlasterHUD()->AddEliminationAnnouncement(Attacker->GetPlayerName(), "themselves");
			return;
		}

		GetBlasterHUD()->AddEliminationAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
	}
}


void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
	ServerCheckMatchState();
}

void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}

ABlasterHUD* ABlasterPlayerController::GetBlasterHUD()
{
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	return BlasterHUD;
}

void ABlasterPlayerController::UpdateTextBlockText(UTextBlock* TextBlock, const int32 Value) const
{
	const auto TextString = FString::Printf(TEXT("%d"), Value);
	UpdateTextBlockText(TextBlock, TextString);
}

void ABlasterPlayerController::UpdateTextBlockText(UTextBlock* TextBlock, const FString Text) const
{
	if (TextBlock)
	{
		TextBlock->SetText(FText::FromString(Text));
	}
}

void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0;
	// ServerTime is since the game started (including time in menu)
	if (MatchState == MatchState::WaitingToStart)
	{
		TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::InProgress)
	{
		TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}
	else if (MatchState == MatchState::Cooldown)
	{
		TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	}

	if (HasAuthority())
	{
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			TimeLeft = BlasterGameMode->GetCountdownTime() + LevelStartingTime;
		}
	}

	const uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	if (CountDownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		else if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
			if (!bCooldownUrgencyPlaying && TimeLeft > 0.f && TimeLeft <= 30.f && GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay && GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay->CooldownUrgency)
			{
				// Should have a flag or something to know if it's already playing and another one to get back to White?
				GetBlasterHUD()->CharacterOverlay->PlayAnimation(GetBlasterHUD()->CharacterOverlay->CooldownUrgency, 0, 30);
				bCooldownUrgencyPlaying = true;
			}
		}
	}
	CountDownInt = SecondsLeft;
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(const float TimeOfClientRequest, const float TimeServerReceivedClientRequest)
{
	const float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = (RoundTripTime * 0.5f);
	const float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(const float TimeOfClientRequest)
{
	const float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);
	}
}

void ABlasterPlayerController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidgetClass)
	{
		ReturnToMainMenuWidget = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidgetClass);
		if (ReturnToMainMenuWidget)
		{
			bReturnToMainMenuWidgetIsShowing = !bReturnToMainMenuWidgetIsShowing;

			if (bReturnToMainMenuWidgetIsShowing)
			{
				ReturnToMainMenuWidget->MenuSetup();
			}
			else
			{
				ReturnToMainMenuWidget->MenuTeardown();
			}
		}
	}
}

void ABlasterPlayerController::HideDeathMessage()
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay &&
		GetBlasterHUD()->CharacterOverlay->DeathMessage &&
		GetBlasterHUD()->CharacterOverlay->KilledBy)
	{
		GetBlasterHUD()->CharacterOverlay->KilledBy->SetVisibility(ESlateVisibility::Collapsed);
		GetBlasterHUD()->CharacterOverlay->DeathMessage->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (const ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn))
	{
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
		SetHUDShield(BlasterCharacter->GetShield(), BlasterCharacter->GetMaxShield());
		if (BlasterCharacter->GetCombatComponent())
		{
			SetHUDGrenades(BlasterCharacter->GetCombatComponent()->GetGrenades());
		}
	}
	HideDeathMessage();
}

void ABlasterPlayerController::HideTeamScores()
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->BlueTeamScore, FString());
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->RedTeamScore, FString());
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->ScoreSpacerText, FString());
	}
}

void ABlasterPlayerController::InitTeamScores()
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->BlueTeamScore, 0);
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->RedTeamScore, 0);
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->ScoreSpacerText, FString("|"));
	}
}

void ABlasterPlayerController::SetHUDBlueTeamScore(const int32 BlueScore)
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->BlueTeamScore, BlueScore);
	}
}

void ABlasterPlayerController::SetHUDRedTeamScore(const int32 RedScore)
{
	if (GetBlasterHUD() &&
		GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->RedTeamScore, RedScore);
	}
}

void ABlasterPlayerController::CheckTimeSync(const float DeltaSeconds)
{
	TimeSyncRunningTime += DeltaSeconds;
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ABlasterPlayerController::HighPingWarning(const bool bShow)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay && GetBlasterHUD()->CharacterOverlay->HighPingImage)
	{
		if (bShow)
		{
			GetBlasterHUD()->CharacterOverlay->HighPingImage->SetOpacity(1.f);
			GetBlasterHUD()->CharacterOverlay->PlayAnimation(GetBlasterHUD()->CharacterOverlay->HighPingAnimation, 0.f, 5);
		}
		else
		{
			GetBlasterHUD()->CharacterOverlay->HighPingImage->SetOpacity(0.f);
			if (HighPingWarningAnimationIsPlaying())
			{
				GetBlasterHUD()->CharacterOverlay->StopAnimation(GetBlasterHUD()->CharacterOverlay->HighPingAnimation);
			}
		}
	}
}

bool ABlasterPlayerController::HighPingWarningAnimationIsPlaying()
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay && GetBlasterHUD()->CharacterOverlay->HighPingAnimation)
	{
		return GetBlasterHUD()->CharacterOverlay->IsAnimationPlaying(GetBlasterHUD()->CharacterOverlay->HighPingAnimation);
	}
	return false;
}

void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}


void ABlasterPlayerController::ClientJoinMiddleOfGame_Implementation(const FName StateOfMatch, const float Warmup, const float Match, const float StartingTime, const float Cooldown)
{
	CooldownTime = Cooldown;
	WarmupTime = Warmup;
	MatchTime = Match;
	LevelStartingTime = StartingTime;
	MatchState = StateOfMatch;

	OnMatchStateSet(MatchState);
	if (GetBlasterHUD() && MatchState == MatchState::WaitingToStart)
	{
		GetBlasterHUD()->AddAnnouncement();
	}
}

void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	if (const ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		CooldownTime = GameMode->CooldownTime;
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		MatchState = GameMode->GetMatchState();

		// Client client to sync values
		ClientJoinMiddleOfGame(MatchState, WarmupTime, MatchTime, LevelStartingTime, CooldownTime);
	}
}

void ABlasterPlayerController::CheckPing(const float DeltaSeconds)
{
	HighPingRunningTime += DeltaSeconds;
	if (HighPingRunningTime > CheckPingFrequency)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// UE_LOG(LogTemp, Warning, TEXT("Ping: %d"), PlayerState->GetCompressedPing() * 4);
			if (PlayerState->GetCompressedPing() * 4 > HighPingThreshold)
			{
				HighPingWarning(true);
				HighPingAnimationRunningTime = 0.f;
				ServerReportPingStatus(true);
			}
			else
			{
				ServerReportPingStatus(false);
			}
		}
		HighPingRunningTime = 0.f;
	}

	if (HighPingWarningAnimationIsPlaying())
	{
		HighPingAnimationRunningTime += DeltaSeconds;
		if (HighPingAnimationRunningTime > HighPingDuration)
		{
			HighPingWarning(false);
		}
	}
}

// The ping is too high
void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

void ABlasterPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	CheckTimeSync(DeltaSeconds);
	SetHUDTime();
	CheckPing(DeltaSeconds);
}

float ABlasterPlayerController::GetServerTime() const
{
	if (HasAuthority())
	{
		return GetWorld()->GetTimeSeconds();
	}
	return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ABlasterPlayerController::OnMatchStateSet(const FName State, const bool bTeamsMatch)
{
	MatchState = State;

	if (MatchState == MatchState::InProgress && GetBlasterHUD())
	{
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress && GetBlasterHUD())
	{
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		HandleCooldown();
	}
}

void ABlasterPlayerController::HandleMatchHasStarted(const bool bTeamsMatch)
{
	if (HasAuthority())
	{
		bShowTeamScores = bTeamsMatch;
	}

	if (GetBlasterHUD())
	{
		GetBlasterHUD()->AddCharacterOverlay();

		if (GetBlasterHUD()->Announcement)
		{
			GetBlasterHUD()->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}

		if (HasAuthority())
		{
			if (bTeamsMatch)
			{
				InitTeamScores();
			}
			else
			{
				HideTeamScores();
			}
		}
	}
}

void ABlasterPlayerController::HandleCooldown()
{
	if (GetBlasterHUD())
	{
		if (GetBlasterHUD()->CharacterOverlay)
		{
			GetBlasterHUD()->CharacterOverlay->RemoveFromParent();
		}

		if (GetBlasterHUD()->Announcement)
		{
			GetBlasterHUD()->Announcement->SetVisibility(ESlateVisibility::Visible);
			UpdateTextBlockText(GetBlasterHUD()->Announcement->AnnouncementText, FString("New Match Starts in:"));

			FString InfoTextString = FString();
			if (const ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this)))
			{
				if (const TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers; TopPlayers.Num() == 0)
				{
					InfoTextString = FString("There is no winner.");
				}
				else if (TopPlayers.Num() == 1 && TopPlayers[0] == GetPlayerState<ABlasterPlayerState>())
				{
					InfoTextString = FString("You are the winner!");
				}
				else if (TopPlayers.Num() == 1)
				{
					InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *TopPlayers[0]->GetPlayerName());
				}
				else if (TopPlayers.Num() > 1)
				{
					InfoTextString = FString("Players tied for the win:\n");
					for (const auto TiedPlayer : TopPlayers)
					{
						InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
					}
				}
			}
			UpdateTextBlockText(GetBlasterHUD()->Announcement->InfoText, InfoTextString);
		}
	}

	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn()))
	{
		BlasterCharacter->bDisableGameplay = true;
		if (BlasterCharacter->GetCombatComponent())
		{
			BlasterCharacter->GetCombatComponent()->FireButtonPressed(false);
		}
	}
}
