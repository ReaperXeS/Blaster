// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"

#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "RocketMovementComponent.h"

AProjectileRocket::AProjectileRocket()
{
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RocketMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

#if WITH_EDITOR
void AProjectileRocket::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (RocketMovementComponent && PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AProjectileRocket, InitialSpeed))
	{
		RocketMovementComponent->InitialSpeed = InitialSpeed;
		RocketMovementComponent->MaxSpeed = InitialSpeed;
	}
}
#endif

void AProjectileRocket::BeginPlay()
{
	Super::BeginPlay();

	SpawnTrailSystem();

	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(ProjectileLoop, GetRootComponent(), NAME_None, GetActorLocation(), EAttachLocation::KeepWorldPosition, false, 1.f, 1.f, 0.f, LoopingSoundAttenuation, nullptr, false);
	}
}

void AProjectileRocket::Destroyed()
{
	// Don't call super, we don't want to destroy the actor
}

void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Ignore self
	if (OtherActor == GetOwner())
	{
		return;
	}

	// Apply Explosion damage
	ApplyExplosionDamage();

	// don't destroy right now
	StartDestroyTimer();

	PlayImpactEffects();

	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}

	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstanceController())
	{
		TrailSystemComponent->GetSystemInstanceController()->Deactivate();
	}

	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}
