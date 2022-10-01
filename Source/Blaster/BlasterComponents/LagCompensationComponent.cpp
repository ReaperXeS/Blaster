// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

// @ServerCall
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package) const
{
	if (GetCharacter())
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		for (auto& BoxPair : GetCharacter()->HitCollisionBoxes)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			Package.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float& HitTime) const
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage;
	InterpFramePackage.Time = HitTime;

	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;
		const FBoxInformation YoungerBoxInfo = YoungerPair.Value;
		const FBoxInformation OlderBoxInfo = OlderFrame.HitBoxInfo[BoxInfoName];

		FBoxInformation InterpBoxInfo;
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.f , InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;
		
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

void ULagCompensationComponent::ShowFramePackage(FFramePackage& Package, const FColor Color) const
{
	for (const auto& BoxPair : Package.HitBoxInfo)
	{
		const FBoxInformation BoxInfo = BoxPair.Value;

		DrawDebugBox(GetWorld(), BoxInfo.Location, BoxInfo.BoxExtent, FQuat(BoxInfo.Rotation), Color, false, 4.f);
	}
}

void ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime)
{
	if (HitCharacter == nullptr
		|| HitCharacter->GetLagCompensationComponent() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr)
	{
		return;
	}
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// Hit Character's Frame History
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;

	// Too far back/too much lag
	if (OldestHistoryTime > HitTime)
	{
		return;
	}
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	// Find the two frames that are closest to the hit time
	while (Older->GetValue().Time > HitTime)
	{
		// March back until: OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr)
		{
			break;
		}
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}

	if (Older->GetValue().Time == HitTime)
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	if (bShouldInterpolate)
	{
		// Interpolate between the two frames
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
}

ABlasterCharacter* ULagCompensationComponent::GetCharacter() const
{
	return Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FrameHistory.Num() > 1)
	{
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		while (HistoryLength >= MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());

			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
	}

	FFramePackage CurrentFrame;
	SaveFramePackage(CurrentFrame);
	FrameHistory.AddHead(CurrentFrame);

	ShowFramePackage(CurrentFrame, FColor::Red);
}
