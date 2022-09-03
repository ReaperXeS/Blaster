// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterPlayerController.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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
		default:
			WeaponTypeString = "Patate";
			break;
		}
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->CarriedAmmoWeaponType, WeaponTypeString);
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

void ABlasterPlayerController::SetHUDScore(const float Score)
{
	if (GetBlasterHUD() && GetBlasterHUD()->CharacterOverlay)
	{
		UpdateTextBlockText(GetBlasterHUD()->CharacterOverlay->ScoreAmount, FMath::FloorToInt(Score));
	}
}

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();
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
	}
	HideDeathMessage();
}
