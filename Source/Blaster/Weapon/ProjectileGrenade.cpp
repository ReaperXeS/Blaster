// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"

#include "GameFrameWork/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectileGrenade::AProjectileGrenade()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->SetIsReplicated(true);
	ProjectileMovementComponent->bShouldBounce = true; // Bouncing projectile
}

void AProjectileGrenade::Destroyed()
{
	ApplyExplosionDamage();

	Super::Destroyed();
}

void AProjectileGrenade::BeginPlay()
{
	// No need of tracer or collision box on hit so we call AActor::BeginPlay() instead of Super::BeginPlay()
	AActor::BeginPlay();

	StartDestroyTimer();
	SpawnTrailSystem();
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// Play a bouncing sound
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BounceSound, GetActorLocation());
	}
}
