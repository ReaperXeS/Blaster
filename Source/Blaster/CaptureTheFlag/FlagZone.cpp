// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"

#include "Blaster/BlasterTypes/Team.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/GameMode/CTFGameMode.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/Flag.h"

AFlagZone::AFlagZone()
{
	PrimaryActorTick.bCanEverTick = false;

	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
}

void AFlagZone::BeginPlay()
{
	Super::BeginPlay();

	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AFlag* OverlappingFlag = Cast<AFlag>(OtherActor); OverlappingFlag && OverlappingFlag->GetTeam() == Team)
	{
		if (const ACTFGameMode* GameMode = Cast<ACTFGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		OverlappingFlag->ResetFlag();
	}
}
