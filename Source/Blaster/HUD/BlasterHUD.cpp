// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterHUD.h"

void ABlasterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;
	if (GEngine) {
		GEngine->GameViewport->GetViewportSize(ViewportSize);
		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		const float SpreadScaled = CrosshairSpreadMax * HUDPackage.CrosshairSpread;
		FVector2D Spread(0.f, 0.f);

		if (HUDPackage.CrosshairsBottom) {
			Spread.X = 0.f;
			Spread.Y = -SpreadScaled;
			DrawCrosshair(HUDPackage.CrosshairsBottom, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairsCenter) {
			Spread.X = 0.f;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsCenter, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairsLeft) {
			Spread.X = -SpreadScaled;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsLeft, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairsRight) {
			Spread.X = SpreadScaled;
			Spread.Y = 0.f;
			DrawCrosshair(HUDPackage.CrosshairsRight, ViewportCenter, Spread);
		}

		if (HUDPackage.CrosshairsTop) {
			Spread.X = 0.f;
			Spread.Y = SpreadScaled;
			DrawCrosshair(HUDPackage.CrosshairsTop, ViewportCenter, Spread);
		}
	}
}

void ABlasterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D TextureDrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f + Spread.Y)
	);

	DrawTexture(Texture, TextureDrawPoint.X, TextureDrawPoint.Y, TextureWidth, TextureHeight, 0.f, 0.f, 1.f, 1.f, FLinearColor::White);
}