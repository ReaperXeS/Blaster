// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	AFlag();

	virtual void Drop() override;
	virtual void BeginPlay() override;
	void ResetFlag();

protected:
	virtual void HandleWeaponStateEquipped() override;
	virtual void HandleWeaponStateDropped() override;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	FTransform InitialTransform;
public:
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
};
