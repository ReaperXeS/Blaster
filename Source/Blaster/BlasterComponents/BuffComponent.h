// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuffComponent();
	friend class ABlasterCharacter;

	void Heal(float HealAmount, float HealingTime);
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffDuration);
	void BuffJump(const float BuffJumpVelocity, const float BuffDuration);
	void SetInitialSpeeds(const float BaseSpeed, const float CrouchSpeed);
	void SetInitialJumpVelocity(const float JumpVelocity);
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealOverTime(float DeltaTime);
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	class ABlasterCharacter* Character;

	/**************************/
	/** Speed Buff Variables **/
	/**************************/
	FTimerHandle SpeedBuffTimer;
	void ResetSpeeds() const;
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BuffBaseSpeed, float BuffCrouchSpeed);

	/**************************/
	/** Jump Buff Variables **/
	/**************************/
	FTimerHandle JumpBuffTimer;
	void ResetJump() const;
	float InitialJumpVelocity;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(const float JumpVelocity);

	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;
};
