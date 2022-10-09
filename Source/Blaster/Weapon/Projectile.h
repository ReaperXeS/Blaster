// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()

public:
	AProjectile();
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	/********************************************/
	/* Used with server-side rewind   		    */
	/********************************************/
	bool bUseServerSideRewind = false;
	FVector_NetQuantize TraceStart;
	FVector_NetQuantize100 InitialVelocity;


	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000.0f;

	// Used only for Grenades and Rockets (bullets is set from weapon)
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	// Used only for Grenades and Rockets (bullets is set from weapon)
	UPROPERTY(EditAnywhere)
	float HeadshotDamage = 40.f;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerFinished();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	void PlayImpactEffects() const;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	void SpawnTrailSystem();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	void ApplyExplosionDamage();
private:
	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	FTimerHandle DestroyTimerHandle;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float ExplosionDamageInnerRadius = 200.0f;

	UPROPERTY(EditAnywhere, Category = "Damage")
	float ExplosionDamageOuterRadius = 500.0f;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
public:
};
