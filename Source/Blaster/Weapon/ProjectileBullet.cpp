// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
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

void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Apply Damage
	if (const auto OwnerCharacter = Cast<ACharacter>(GetOwner()))
	{
		if (const auto OwnerController = OwnerCharacter->Controller)
		{
			const auto HitDirection = (Hit.Location - GetActorLocation()).GetSafeNormal();
			UGameplayStatics::ApplyPointDamage(OtherActor, Damage, HitDirection, Hit, OwnerController, this, UDamageType::StaticClass());
		}
	}

	// Will be destroyed here
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();

	FPredictProjectilePathParams PredictParams;
	PredictParams.bTraceWithChannel = true;
	PredictParams.bTraceWithCollision = true;
	PredictParams.DrawDebugTime = 5.f;
	PredictParams.DrawDebugType = EDrawDebugTrace::ForDuration;
	PredictParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;
	PredictParams.MaxSimTime = 4.f;
	PredictParams.ProjectileRadius = 5.f;
	PredictParams.SimFrequency = 30.f;
	PredictParams.StartLocation = GetActorLocation();
	PredictParams.TraceChannel = ECC_Visibility;
	PredictParams.ActorsToIgnore.Add(this);

	FPredictProjectilePathResult PredictResult;

	UGameplayStatics::PredictProjectilePath(this, PredictParams, PredictResult);
}
