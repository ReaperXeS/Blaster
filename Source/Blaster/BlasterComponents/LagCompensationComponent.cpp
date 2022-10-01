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

	FFramePackage Package;
	SaveFramePackage(Package);

	ShowFramePackage(Package, FColor::Orange);
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

void ULagCompensationComponent::ShowFramePackage(FFramePackage& Package, const FColor Color) const
{
	for (const auto& BoxPair : Package.HitBoxInfo)
	{
		const FBoxInformation BoxInfo = BoxPair.Value;

		DrawDebugBox(GetWorld(), BoxInfo.Location, BoxInfo.BoxExtent, FQuat(BoxInfo.Rotation), Color, true);
	}
}

ABlasterCharacter* ULagCompensationComponent::GetCharacter() const
{
	return Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
}

void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
