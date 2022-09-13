// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) const
{
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size()),
		FColor::Cyan,
		true);

	return FVector(TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size());
}

void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}

	if (const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash"); MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();
		const FVector End = Start + (HitTarget - Start) * 1.25f;

		if (UWorld* World = GetWorld())
		{
			FHitResult FireHit;
			World->LineTraceSingleByChannel(FireHit, Start, End, ECC_Visibility);
			FVector BeamEnd = End;
			if (FireHit.bBlockingHit)
			{
				ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
				AController* OwnerController = OwnerPawn->GetController();
				if (HitCharacter && HasAuthority() && OwnerController)
				{
					UGameplayStatics::ApplyPointDamage(HitCharacter, Damage, FireHit.ImpactPoint, FireHit, OwnerController, this, UDamageType::StaticClass());
				}

				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
				}

				BeamEnd = FireHit.ImpactPoint;
				if (HitSound) // Hit sound is optional
				{
					UGameplayStatics::PlaySoundAtLocation(World, HitSound, FireHit.ImpactPoint);
				}
			}

			if (BeamParticles) // Beam particles are optional
			{
				if (const auto BeamParticleSystemComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, SocketTransform))
				{
					BeamParticleSystemComp->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}

			if (MuzzleFlash) // Muzzle flash is optional
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, MuzzleFlash, SocketTransform);
			}

			if (FireSound) // Fire sound is optional
			{
				UGameplayStatics::PlaySoundAtLocation(World, FireSound, GetActorLocation());
			}
		}
	}
}
