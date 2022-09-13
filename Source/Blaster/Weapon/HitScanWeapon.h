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

	UPROPERTY(EditAnywhere, Category = "Weapon")
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	USoundCue* FireSound;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	USoundCue* HitSound;

	/*********************************
	 * Trace end with scatter
	 * ******************************/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
protected:
	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const;
public:
	virtual void Fire(const FVector& HitTarget) override;
};
