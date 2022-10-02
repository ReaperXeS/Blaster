// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	ABlasterCharacter* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;

	UPROPERTY()
	FHitResult HitResult;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;

	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	friend class ABlasterCharacter;

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(FFramePackage& Package, FColor Color) const;
	FServerSideRewindResult ServerSideRewind(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime) const;

	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(class ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, const float& HitTime, class AWeapon* DamageCauser);

	// TODO: Not used because we do damage for each pellet hit
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float& HitTime);
protected:
	virtual void BeginPlay() override;
	void SaveFramePackage(FFramePackage& Package) const;
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, const float& HitTime) const;
	FServerSideRewindResult ConfirmHit(const FFramePackage& HitFrame, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation) const;
	static void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	static void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& FramePackage, bool bResetPosition);
	static void EnableCharacterMeshCollision(const ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);

	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, const float& HitTime) const;

	/************************/
	/* Shotguns				*/
	/************************/
	// TODO: Not used because we do damage for each pellet hit
	FShotgunServerSideRewindResult ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, const float& HitTime) const;
	// TODO: Not used because we do damage for each pellet hit
	FShotgunServerSideRewindResult ConfirmShotgunHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations) const;

	ABlasterCharacter* GetCharacter() const;
	void SaveFramePackage();
private:
	UPROPERTY()
	ABlasterCharacter* Character;

	UPROPERTY()
	class ABlasterPlayerController* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	// Max Record Time in Seconds
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:
};
