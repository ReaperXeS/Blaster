// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterHUD.h"

#include "CharacterOverlay.h"
#include "Announcement.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"

void ABlasterHUD::BeginPlay()
{
	Super::BeginPlay();

	if (const auto PlayerController = GetOwningPlayerController(); PlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
	}
}

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (GEngine)
	{
		FVector2D ViewportSize;
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		FVector2D Spread;

		if (HUDPackage.CrosshairsBottom)
		{
			Spread.X = 0.f;
			Spread.Y = -SpreadScaled;
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsCenter)
		{
			Spread.X = 0.f;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsLeft)
		{
			Spread.X = -SpreadScaled;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsRight)
		{
			Spread.X = SpreadScaled;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}

		if (HUDPackage.CrosshairsTop)
		{
			Spread.X = 0.f;
			Spread.Y = SpreadScaled;
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread, HUDPackage.CrosshairsColor);
		}
	}
}

void ABlasterHUD::AddAnnouncement()
{
	if (const auto PlayerController = GetOwningPlayerController(); PlayerController && AnnouncementClass)
	{
		Announcement = CreateWidget<UAnnouncement>(PlayerController, AnnouncementClass);
		Announcement->AddToViewport();
	}
}

void ABlasterHUD::AddEliminationAnnouncement(const FString AttackerName, const FString VictimName)
{
	OwningPlayer = OwningPlayer == nullptr ? GetOwningPlayerController() : OwningPlayer;
	if (OwningPlayer && EliminationAnnouncementClass)
	{
		const auto EliminationAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayer, EliminationAnnouncementClass);
		EliminationAnnouncementWidget->SetEliminationAnnouncement(AttackerName, VictimName);
		EliminationAnnouncementWidget->AddToViewport();

		// Move up old announcements up
		for (const UElimAnnouncement* EliminationAnnouncement : EliminationAnnouncements)
		{
			if (EliminationAnnouncement && EliminationAnnouncement->AnnouncementBox)
			{
				if (UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(EliminationAnnouncement->AnnouncementBox))
				{
					const FVector2d Position = CanvasSlot->GetPosition();
					CanvasSlot->SetPosition(FVector2d(Position.X, Position.Y - CanvasSlot->GetSize().Y));
				}
			}
		}
		EliminationAnnouncements.Add(EliminationAnnouncementWidget);

		FTimerHandle EliminationTimer;
		FTimerDelegate TimerDelegate;

		TimerDelegate.BindUFunction(this, FName("EliminationAnnouncementTimerFinished"), EliminationAnnouncementWidget);
		GetWorldTimerManager().SetTimer(EliminationTimer, TimerDelegate, EliminationAnnouncementTime, false);
	}
}

void ABlasterHUD::AddCharacterOverlay() const
{
	if (CharacterOverlay)
	{
		CharacterOverlay->AddToViewport();
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f + Spread.Y)
	);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, CrosshairColor);
}

void ABlasterHUD::EliminationAnnouncementTimerFinished(UElimAnnouncement* MessageToRemove)
{
	if (MessageToRemove)
	{
		MessageToRemove->RemoveFromParent();
	}
}
