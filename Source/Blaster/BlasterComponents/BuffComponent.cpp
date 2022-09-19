// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"

#include "Blaster/Character/BlasterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UBuffComponent::Heal(const float HealAmount, const float HealingTime)
{
	bHealing = true;
	HealingRate = HealAmount / HealingTime;
	AmountToHeal += HealAmount;
}

void UBuffComponent::HealOverTime(float DeltaTime)
{
	if (bHealing && Character && !Character->IsEliminated())
	{
		const float HealThisFrame = HealingRate * DeltaTime;
		Character->Heal(HealThisFrame);
		AmountToHeal -= HealThisFrame;
		if (AmountToHeal <= 0 || Character->GetHealth() >= Character->GetMaxHealth())
		{
			bHealing = false;
			AmountToHeal = 0;
		}
	}
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealOverTime(DeltaTime);
}
