// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"

#include "MultiplayerSessionsSubsystem.h"
#include "Components/Button.h"
#include "GameFramework/GameModeBase.h"

void UReturnToMainMenu::UpdateInputMode(const FInputModeDataBase& InData, const bool bShowMouseCursor)
{
	if (const UWorld* World = GetWorld())
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			PlayerController->SetInputMode(InData);
			PlayerController->SetShowMouseCursor(bShowMouseCursor);
		}
	}
}

void UReturnToMainMenu::MenuSetup()
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	FInputModeGameAndUI InputMode;
	InputMode.SetWidgetToFocus(TakeWidget());
	UpdateInputMode(InputMode, true);

	if (ReturnToMainMenuButton && !ReturnToMainMenuButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnToMainMenuButtonClicked))
	{
		ReturnToMainMenuButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnToMainMenuButtonClicked);
	}

	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem && !MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &UReturnToMainMenu::OnDestroySession))
		{
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

bool UReturnToMainMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	return true;
}

void UReturnToMainMenu::OnDestroySession(const bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnToMainMenuButton->SetIsEnabled(true);
		return;
	}
	if (const UWorld* World = GetWorld())
	{
		if (AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>())
		{
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			// We are on the client
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText::FromString("Session Destroyed"));
			}
		}
	}
}

void UReturnToMainMenu::MenuTeardown()
{
	RemoveFromParent();
	const FInputModeGameOnly InputMode;
	UpdateInputMode(InputMode, false);

	if (ReturnToMainMenuButton && ReturnToMainMenuButton->OnClicked.IsAlreadyBound(this, &UReturnToMainMenu::ReturnToMainMenuButtonClicked))
	{
		ReturnToMainMenuButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnToMainMenuButtonClicked);
	}

	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsAlreadyBound(this, &UReturnToMainMenu::OnDestroySession))
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

void UReturnToMainMenu::ReturnToMainMenuButtonClicked()
{
	ReturnToMainMenuButton->SetIsEnabled(false);
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->DestroySession();
	}
}
