// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Menu.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;
	virtual void OnLevelRemovedFromWorld(ULevel* aLevel, UWorld* aWorld) override;

	/*
	* Callbacks for the custom delegates on the MultiplayerSessionsSubsystem. Should be a UFUNCTION since it's a multicast delegate
	*/
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);

	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSucessful);

	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);
public:
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 aNumbPublicConnections = 4, FString NewMatchType = TEXT("FreeForAll"), FString LobbyPath = TEXT(""));

private:
	UPROPERTY(meta = (BindWidget))
	class UButton* btnHost;

	UPROPERTY(meta = (BindWidget))
	class UButton* btnJoin;

	UFUNCTION()
	void btnHostClick();

	UFUNCTION()
	void btnJoinClick();

	void MenuTearDown();

	// Subsystem designed to handle online session functionality
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumbPublicConnections{4};

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("FreeForAll")};

	FString PathToLobby{TEXT("")};
};
