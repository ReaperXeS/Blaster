// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"

#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHitResult) const
{
	if (const UWorld* World = GetWorld())
	{
		const FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(OutHitResult, TraceStart, End, ECC_Visibility);

		FVector BeamEnd = End;
		if (OutHitResult.bBlockingHit)
		{
			BeamEnd = OutHitResult.ImpactPoint;
		}
		// draw debug sphere
		DrawDebugSphere(World, BeamEnd, 16.f, 12, FColor::Orange, true);

		if (BeamParticles) // Beam particles are optional
		{
			if (const auto BeamParticleSystemComp = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, true))
			{
				BeamParticleSystemComp->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
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

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (UWorld* World = GetWorld(); World && FireHit.bBlockingHit && HitCharacter && GetBlasterOwnerController())
		{
			if (bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled(); HasAuthority() && bCauseAuthDamage)
			{
				UGameplayStatics::ApplyPointDamage(HitCharacter, Damage, FireHit.ImpactPoint, FireHit, GetBlasterOwnerController(), this, UDamageType::StaticClass());
			}

			if (!HasAuthority() && bUseServerSideRewind && GetBlasterOwnerCharacter() && GetBlasterOwnerCharacter()->GetLagCompensationComponent())
			{
				GetBlasterOwnerCharacter()->GetLagCompensationComponent()->ServerScoreRequest(HitCharacter, Start, HitTarget, GetBlasterOwnerController()->GetServerTime() - GetBlasterOwnerController()->SingleTripTime, this);
			}

			if (ImpactParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
			}

			if (HitSound) // Hit sound is optional
			{
				UGameplayStatics::PlaySoundAtLocation(World, HitSound, FireHit.ImpactPoint);
			}
		}
	}
}
