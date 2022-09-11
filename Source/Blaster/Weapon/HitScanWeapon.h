// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "Weapon")
	float Damage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	class UParticleSystem* ImpactParticles;
public:
	virtual void Fire(const FVector& HitTarget) override;
};
