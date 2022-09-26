// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Components/ActorComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "CombatComponent.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	void UpdateAmmoValues();
	void UpdateShotgunAmmoValues();
	void UpdateHUDGrenades();
	void FireButtonPressed(bool aIsPressed);

	void ShotgunShellReload();

	void JumpToShotgunEnd();
	void ThrowGrenade();

	void ThrowGrenadeFinished();
	void LaunchGrenade() const;
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target) const;
	void ShowGrenade(const bool bShow) const;

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	void PickupAmmo(const EWeaponType WeaponType, int32 AmmoAmount);
protected:
	virtual void BeginPlay() override;
	void DropEquippedWeapon() const;
	void AttachActorToLeftHand(AActor* ActorToAttached) const;
	void AttachActorToRightHand(AActor* ActorToAttached) const;
	void AttachActorToBackpack(AActor* ActorToAttached) const;
	void UpdateCarriedAmmo(const int32 AmountToAdd);
	void PlayEquipWeaponSound(const AWeapon* WeaponToEquip) const;
	void ReloadIfEmpty();


	void SetAiming(bool aIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool aIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon() const;

	UFUNCTION()
	void OnRep_SecondaryWeapon() const;

	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	// Executed on Server and Client
	void HandleReload() const;

	int32 AmountToReload() const;

	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
	UPROPERTY()
	class ABlasterCharacter* Character;
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	UPROPERTY()
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFireButtonPressed;

	/**
	* HUD and crosshairs
	*/
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairShootingFactor;
	float CrosshairEnemyTargetedFactor;
	FVector HitTarget;
	FHUDPackage HUDPackage;

	// bool flag to know if the target is an enemy (Implement UInteractWithCrosshairsInterface)
	bool bTargetIsAnEnemy;

	/**
	 * Aiming and FOV
	 */
	// Field of View while not aiming; set to camera's base FOV in BeginPlay
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	/**
	 * Automatic Fire
	 */
	FTimerHandle FireTimer;
	bool bCanFire = true;

	void StartFireTimer();
	void Fire();
	void FireTimerFinished();

	bool CanFire() const;

	// Carried Ammo for the currently equipped weapon
	UPROPERTY(ReplicatedUsing=OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UPROPERTY(EditAnywhere)
	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	UPROPERTY(ReplicatedUsing=OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 3;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere, Category = "Combat")
	int32 MaxGrenades = 4;
public:
	void FinishReloading();

	FORCEINLINE int32 GetGrenades() const { return Grenades; }
};
