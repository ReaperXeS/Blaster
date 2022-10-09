// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"

#include "Blaster/Blaster.h"
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

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float& HitTime) const
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	FFramePackage InterpFramePackage = FFramePackage();
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
	// Disable Collision on Mesh
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable Collision for Head first
	if (UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")])
	{
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (UWorld* World = GetWorld())
		{
			FHitResult HeadHitResult;
			World->LineTraceSingleByChannel(HeadHitResult, TraceStart, TraceEnd, ECC_HitBox);
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
					BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
				}
			}

			World->LineTraceSingleByChannel(HeadHitResult, TraceStart, TraceEnd, ECC_HitBox);
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

FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& HitFrame, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity) const
{
	if (HitCharacter == nullptr)
	{
		return FServerSideRewindResult();
	}

	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	MoveBoxes(HitCharacter, HitFrame, false);
	// Disable Collision on Mesh
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// Enable Collision for Head first
	if (UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")])
	{
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);

		FPredictProjectilePathParams PredictParams;
		PredictParams.bTraceWithChannel = true;
		PredictParams.bTraceWithCollision = true;
		PredictParams.LaunchVelocity = InitialVelocity;
		PredictParams.MaxSimTime = MaxRecordTime;
		PredictParams.ProjectileRadius = 5.f;
		PredictParams.SimFrequency = 15.f;
		PredictParams.StartLocation = TraceStart;
		PredictParams.TraceChannel = ECC_HitBox;
		PredictParams.ActorsToIgnore.Add(GetOwner());

		FPredictProjectilePathResult PredictResult;

		UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
		if (PredictResult.HitResult.bBlockingHit)
		{
			MoveBoxes(HitCharacter, CurrentFrame, true);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, true, PredictResult.HitResult};
		}
		// Didn't hit head, check the rest
		for (auto& BoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (BoxPair.Value)
			{
				BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
			}
		}

		UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
		if (PredictResult.HitResult.bBlockingHit)
		{
			MoveBoxes(HitCharacter, CurrentFrame, true);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			return FServerSideRewindResult{true, false, PredictResult.HitResult};
		}
	}

	MoveBoxes(HitCharacter, CurrentFrame, true);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	return FServerSideRewindResult{false, false, FHitResult()};
}

FShotgunServerSideRewindResult ULagCompensationComponent::ConfirmShotgunHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const
{
	FShotgunServerSideRewindResult Result;
	TArray<FFramePackage> CurrentFrames;

	// Cache positions, move boxes, and disable collision
	for (FFramePackage FramePackage : FramePackages)
	{
		FFramePackage CurrentFrame;

		CacheBoxPositions(FramePackage.Character, CurrentFrame);
		MoveBoxes(FramePackage.Character, FramePackage, false);
		EnableCharacterMeshCollision(FramePackage.Character, ECollisionEnabled::NoCollision);
		CurrentFrame.Character = FramePackage.Character;

		CurrentFrames.Add(CurrentFrame);
	}

	for (FFramePackage FramePackage : FramePackages)
	{
		// Enable Collision for Head first
		if (UBoxComponent* HeadBox = FramePackage.Character->HitCollisionBoxes[FName("head")]; FramePackage.Character && HeadBox)
		{
			HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
		}
	}

	if (UWorld* World = GetWorld())
	{
		// Check for Head Shots
		for (auto HitLocation : HitLocations)
		{
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
			FHitResult HeadHitResult;
			World->LineTraceSingleByChannel(HeadHitResult, TraceStart, TraceEnd, ECC_HitBox);
			if (HeadHitResult.bBlockingHit)
			{
				if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(HeadHitResult.GetActor()))
				{
					if (Result.HeadShots.Contains(HitCharacter))
					{
						Result.HeadShots[HitCharacter]++;
					}
					else
					{
						Result.HeadShots.Emplace(HitCharacter, 1);
					}
				}
			}
		}

		// Enable collision on all hit boxes and disable for head box
		for (auto& Frame : FramePackages)
		{
			if (Frame.Character)
			{
				for (auto& BoxPair : Frame.Character->HitCollisionBoxes)
				{
					if (BoxPair.Value)
					{
						BoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
						BoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECR_Block);
					}
				}

				// Disable collision for Head
				if (UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")])
				{
					HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				}
			}
		}

		// Check for Body Shots
		for (auto HitLocation : HitLocations)
		{
			const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
			FHitResult BodyHitResult;
			World->LineTraceSingleByChannel(BodyHitResult, TraceStart, TraceEnd, ECC_HitBox);
			if (BodyHitResult.bBlockingHit)
			{
				if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(BodyHitResult.GetActor()))
				{
					if (Result.BodyShots.Contains(HitCharacter))
					{
						Result.BodyShots[HitCharacter]++;
					}
					else
					{
						Result.BodyShots.Emplace(HitCharacter, 1);
					}
				}
			}
		}

		for (auto Frame : CurrentFrames)
		{
			MoveBoxes(Frame.Character, Frame, true);
			EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
		}
	}

	return Result;
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
			if (FramePackage.HitBoxInfo.Contains(BoxPair.Key))
			{
				BoxPair.Value->SetWorldLocation(FramePackage.HitBoxInfo[BoxPair.Key].Location);
				BoxPair.Value->SetWorldRotation(FramePackage.HitBoxInfo[BoxPair.Key].Rotation);
				BoxPair.Value->SetBoxExtent(FramePackage.HitBoxInfo[BoxPair.Key].BoxExtent);
			}

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

FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, const float& HitTime) const
{
	if (HitCharacter == nullptr
		|| HitCharacter->GetLagCompensationComponent() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr
		|| HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr)
	{
		return FFramePackage();
	}

	// Hit Character's Frame History
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;

	// Too far back/too much lag
	if (OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}
	if (OldestHistoryTime == HitTime)
	{
		return History.GetTail()->GetValue();
	}

	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	if (NewestHistoryTime <= HitTime)
	{
		return History.GetHead()->GetValue();
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
		return Older->GetValue();
	}

	// Interpolate between the two frames
	FFramePackage Return = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	Return.Character = HitCharacter;
	return Return;
}

FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime) const
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const float& HitTime) const
{
	const FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);

	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity);
}

FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float& HitTime) const
{
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	return ConfirmShotgunHit(FramesToCheck, TraceStart, HitLocations);
}

void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime)
{
	if (const FServerSideRewindResult Result = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime); Character && Character->GetEquippedWeapon() && Result.bHitConfirmed)
	{
		const float DamageToCause = Result.bHeadShot ? Character->GetEquippedWeapon()->GetHeadshotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyPointDamage(HitCharacter, DamageToCause, HitLocation, Result.HitResult, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, const float& HitTime)
{
	if (const FServerSideRewindResult Result = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime); HitCharacter && Character && Character->GetEquippedWeapon() && Result.bHitConfirmed)
	{
		const float DamageToCause = Result.bHeadShot ? Character->GetEquippedWeapon()->GetHeadshotDamage() : Character->GetEquippedWeapon()->GetDamage();
		UGameplayStatics::ApplyPointDamage(HitCharacter, DamageToCause, Result.HitResult.ImpactPoint, Result.HitResult, Character->Controller, Character->GetEquippedWeapon(), UDamageType::StaticClass());
	}
}

void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float& HitTime)
{
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);
	for (auto& HitCharacter : HitCharacters)
	{
		if (HitCharacter && HitCharacter->GetEquippedWeapon())
		{
			const float HeadShotDamage = Confirm.HeadShots.Contains(HitCharacter) ? Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadshotDamage() : 0.f;
			const float BodyDamage = Confirm.BodyShots.Contains(HitCharacter) ? Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage() : 0.f;
			const float DamageToCause = HeadShotDamage + BodyDamage;

			UGameplayStatics::ApplyPointDamage(HitCharacter, DamageToCause, HitCharacter->GetActorLocation(), FHitResult(), Character->Controller, HitCharacter->GetEquippedWeapon(), UDamageType::StaticClass());
		}
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

// @ServerCall
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package) const
{
	if (GetCharacter())
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = GetCharacter();
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

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->HasAuthority())
	{
		SaveFramePackage();
	}
}
