// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"

#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

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
