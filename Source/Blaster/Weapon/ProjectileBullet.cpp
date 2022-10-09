// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFrameWork/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (ProjectileMovementComponent && PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		ProjectileMovementComponent->InitialSpeed = InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = InitialSpeed;
	}
}
#endif

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Apply Damage
	if (const auto OwnerCharacter = Cast<ABlasterCharacter>(GetOwner()))
	{
		if (const auto OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller))
		{
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			const auto HitDirection = (Hit.Location - GetActorLocation()).GetSafeNormal();
			if (OwnerCharacter->HasAuthority() && (!bUseServerSideRewind || OwnerCharacter->IsLocallyControlled()) && HitCharacter)
			{
				const float DamageToCause = Hit.BoneName.ToString() == HitCharacter->HeadBoneName ? HeadshotDamage : Damage;
				UGameplayStatics::ApplyPointDamage(HitCharacter, DamageToCause, HitDirection, Hit, OwnerController, this, UDamageType::StaticClass());
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}

			if (bUseServerSideRewind && HitCharacter && OwnerCharacter->GetLagCompensationComponent() && OwnerCharacter->IsLocallyControlled())
			{
				OwnerCharacter->GetLagCompensationComponent()->ProjectileServerScoreRequest(HitCharacter, TraceStart, InitialVelocity, OwnerController->GetServerTime() - OwnerController->SingleTripTime);
			}
		}
	}

	// Will be destroyed here
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	// FPredictProjectilePathParams PredictParams;
	// PredictParams.bTraceWithChannel = true;
	// PredictParams.bTraceWithCollision = true;
	// PredictParams.DrawDebugTime = 5.f;
	// PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	// PredictParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	// PredictParams.MaxSimTime = 4.f;
	// PredictParams.ProjectileRadius = 5.f;
	// PredictParams.SimFrequency = 30.f;
	// PredictParams.StartLocation = GetActorLocation();
	// PredictParams.TraceChannel = ECC_Visibility;
	// PredictParams.ActorsToIgnore.Add(this);
	//
	// FPredictProjectilePathResult PredictResult;
	//
	// UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
}
