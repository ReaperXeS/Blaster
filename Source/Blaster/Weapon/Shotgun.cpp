// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

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

		// Maps hit character to number of hits
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult HitResult;
			WeaponTraceHit(Start, HitTarget, HitResult);

			if (HitResult.bBlockingHit)
			{
				if (ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(HitResult.GetActor()); HitCharacter)
				{
					if (OwnerPawn->HasAuthority() && (!bUseServerSideRewind || OwnerPawn->IsLocallyControlled()))
					{
						UGameplayStatics::ApplyPointDamage(HitCharacter, Damage, HitResult.ImpactPoint, HitResult, OwnerController, this, UDamageType::StaticClass());
					}

					if (!HasAuthority() && bUseServerSideRewind && GetBlasterOwnerCharacter() && GetBlasterOwnerCharacter()->GetLagCompensationComponent() && OwnerPawn->IsLocallyControlled())
					{
						GetBlasterOwnerCharacter()->GetLagCompensationComponent()->ServerScoreRequest(HitCharacter, Start, HitTarget, GetBlasterOwnerController()->GetServerTime() - GetBlasterOwnerController()->SingleTripTime, this);
					}
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

void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr)
	{
		return;
	}
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();
	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;

	for (uint32 i = 0; i < NumberOfPellets; ++i)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		const FVector EndLoc = SphereCenter + RandVec;
		FVector ToEndLoc = EndLoc - Start;
		ToEndLoc = FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());

		HitTargets.Add(ToEndLoc);
	}
}
