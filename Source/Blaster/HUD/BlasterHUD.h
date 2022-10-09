// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere)
	class UTexture2D* CrosshairsCenter = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsLeft = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsRight = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsTop = nullptr;

	UPROPERTY(EditAnywhere)
	UTexture2D* CrosshairsBottom = nullptr;
	float CrosshairSpread = 0.f;
	FLinearColor CrosshairsColor;
};

/**
 *
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	virtual void DrawHUD() override;

	/*************************************
	*		Character Overlay
	**************************************/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	void AddCharacterOverlay() const;

	/*************************************
	*		Announcement Overlay
	**************************************/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY()
	class UAnnouncement* Announcement;

	void AddAnnouncement();
	void AddEliminationAnnouncement(const FString AttackerName, const FString VictimName);
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY()
	class APlayerController* OwningPlayer;
	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> EliminationAnnouncementClass;

	UPROPERTY(EditAnywhere)
	float EliminationAnnouncementTime = 2.5f;

	UFUNCTION()
	static void EliminationAnnouncementTimerFinished(UElimAnnouncement* MessageToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> EliminationAnnouncements;
public:
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
