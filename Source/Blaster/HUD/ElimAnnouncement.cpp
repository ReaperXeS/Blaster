// Fill out your copyright notice in the Description page of Project Settings.


#include "ElimAnnouncement.h"
#include "Components/TextBlock.h"

void UElimAnnouncement::SetEliminationAnnouncement(const FString AttackerName, const FString VictimName) const
{
	if (AnnouncementText)
	{
		const FString Announcement = FString::Printf(TEXT("%s killed %s"), *AttackerName, *VictimName);
		AnnouncementText->SetText(FText::FromString(Announcement));
	}
}
