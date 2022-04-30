// Fill out your copyright notice in the Description page of Project Settings.

#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::SetDisplayText(FString aTextToDisplay)
{
	if (DisplayText) {
		DisplayText->SetText(FText::FromString(aTextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* aPawn)
{
	ENetRole RemoteRole = aPawn->GetRemoteRole();
	FString Role;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority");
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("AutonomousProxy");
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("SimulatedProxy");
		break;
	case ENetRole::ROLE_None:
		Role = FString("None");
		break;
	}

	FString RoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	SetDisplayText(RoleString);
}

void UOverheadWidget::ShowPlayerName(APawn* aPawn)
{
	if (aPawn && aPawn->GetPlayerState()) {
		SetDisplayText(aPawn->GetPlayerState()->GetPlayerName());
	}
}

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}