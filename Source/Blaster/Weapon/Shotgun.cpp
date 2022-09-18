// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	const UWorld* World = GetWorld();
	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"); MuzzleFlashSocket && World)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		AController* OwnerController = OwnerPawn->GetController();
		for (uint32 i = 0; i < NumberOfPellets; ++i)
		{
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);

			if (HitResult.bBlockingHit)
			{
				if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()); HasAuthority() && HitCharacter)
				{
					UGameplayStatics::ApplyPointDamage(HitCharacter, Damage, HitResult.ImpactPoint, HitResult, OwnerController, this, UDamageType::StaticClass());
				}

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
				}

				if (HitSound) // Hit sound is optional
				{
					UGameplayStatics::PlaySoundAtLocation(World, HitSound, HitResult.ImpactPoint, 0.5f, FMath::FRandRange(-.5f, .5f));
				}
			}
		}
	}
}
