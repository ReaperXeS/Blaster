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
protected:
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

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult) const;
public:
	virtual void Fire(const FVector& HitTarget) override;
};
