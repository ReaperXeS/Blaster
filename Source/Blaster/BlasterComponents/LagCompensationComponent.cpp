// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Weapon/Weapon.h"

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
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBoxInfo.Location, YoungerBoxInfo.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBoxInfo.Rotation, YoungerBoxInfo.Rotation, 1.f, InterpFraction);
		InterpBoxInfo.BoxExtent = YoungerBoxInfo.BoxExtent;

		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	return InterpFramePackage;
}

void ULagCompensationComponent::EnableCharacterMeshCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	if (HitCharacter && HitCharacter->GetMesh())
	{
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& HitFrame, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const
{
	if (HitCharacter == nullptr)
	{
		return FServerSideRewindResult();
	}

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, HitFrame, false);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Disable Collision on Mesh

	// Enable Collision for Head first
	if (UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")])
	{
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (UWorld* World = GetWorld())
		{
			FHitResult HeadHitResult;
			World->LineTraceSingleByChannel(HeadHitResult, TraceStart, TraceEnd, ECC_Visibility);
			if (HeadHitResult.bBlockingHit)
			{
				MoveBoxes(HitCharacter, CurrentFrame, true);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, true, HeadHitResult};
			}
			// Didn't hit head, check the rest
			for (auto& BoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (BoxPair.Value)
				{
					BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
					BoxPair.Value->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
				}
			}

			World->LineTraceSingleByChannel(HeadHitResult, TraceStart, TraceEnd, ECC_Visibility);
			if (HeadHitResult.bBlockingHit)
			{
				MoveBoxes(HitCharacter, CurrentFrame, true);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				return FServerSideRewindResult{true, false, HeadHitResult};
			}
		}
	}

	MoveBoxes(HitCharacter, CurrentFrame, true);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false, FHitResult()};
}

void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	if (HitCharacter == nullptr)
	{
		return;
	}

	for (auto BoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (BoxPair.Value != nullptr)
		{
			FBoxInformation BoxInfo;
			BoxInfo.Location = BoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = BoxPair.Value->GetScaledBoxExtent();

			OutFramePackage.HitBoxInfo.Add(BoxPair.Key, BoxInfo);
		}
	}
}

void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage, const bool bResetPosition)
{
	if (HitCharacter == nullptr)
	{
		return;
	}

	for (auto& BoxPair : HitCharacter->HitCollisionBoxes)
	{
		if (BoxPair.Value != nullptr)
		{
			BoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfo[BoxPair.Key].Location);
			BoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfo[BoxPair.Key].Rotation);
			BoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfo[BoxPair.Key].BoxExtent);

			if (bResetPosition)
			{
				BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}
}

void ULagCompensationComponent::ShowFramePackage(FFramePackage& Package, const FColor Color) const
{
	for (const auto& BoxPair : Package.HitBoxInfo)
	{
		const FBoxInformation BoxInfo = BoxPair.Value;

		DrawDebugBox(GetWorld(), BoxInfo.Location, BoxInfo.BoxExtent, FQuat(BoxInfo.Rotation), Color, false, 4.f);
	}
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime)
{
	if (HitCharacter == nullptr
		|| HitCharacter->GetLagCompensationComponent() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr)
	{
		return FServerSideRewindResult();
	}
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;

	// Hit Character's Frame History
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;

	// Too far back/too much lag
	if (OldestHistoryTime > HitTime)
	{
		return FServerSideRewindResult();
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

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime, class AWeapon* DamageCauser)
{
	const FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);
	if (Character && HitCharacter && Result.bHitConfirmed)
	{
		UGameplayStatics::ApplyPointDamage(HitCharacter, DamageCauser->GetDamage(), HitLocation, Result.HitResult, Character->Controller, DamageCauser, UDamageType::StaticClass());
		// if (Result.bHeadShot)
		// {
		// 	// Headshot
		// 	HitCharacter->ServerScoreRequest(DamageCauser, 100);
		// }
		// else
		// {
		// 	// Bodyshot
		// 	HitCharacter->ServerScoreRequest(DamageCauser, 50);
		// }
	}
}

ABlasterCharacter* ULagCompensationComponent::GetCharacter() const
{
	return Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
}

void ULagCompensationComponent::SaveFramePackage()
{
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
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->HasAuthority())
	{
		SaveFramePackage();
	}
}
