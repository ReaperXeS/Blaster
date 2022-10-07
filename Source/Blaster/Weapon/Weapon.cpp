// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Casing.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	AActor::SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::EnableCustomDepth(const bool bEnabled) const
{
	if (WeaponMesh)
	{
		WeaponMesh->SetRenderCustomDepth(bEnabled);
		WeaponMesh->SetCustomDepthStencilValue(1);
	}
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	AreaSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget) const
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr)
	{
		return FVector();
	}
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector Start = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - Start).GetSafeNormal();
	const FVector SphereCenter = Start + ToTargetNormalized * DistanceToSphere;
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - Start;

	// DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	// DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	// DrawDebugLine(GetWorld(),TraceStart,FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()), FColor::Cyan,true);

	return FVector(Start + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor))
	{
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::SetEnablePhysics(const bool Enabled) const
{
	WeaponMesh->SetSimulatePhysics(Enabled);
	WeaponMesh->SetEnableGravity(Enabled);
	WeaponMesh->SetCollisionEnabled(Enabled ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
}


void AWeapon::SpendRound()
{
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

	// Sent Authoritative value to client
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		SequenceAmmoRequest++;
	}
}

void AWeapon::ClientUpdateAmmo_Implementation(const int32 ServerAmmo)
{
	if (HasAuthority())
	{
		return;
	}

	Ammo = ServerAmmo; // Sync with server
	SequenceAmmoRequest--;
	Ammo -= SequenceAmmoRequest;
	SetHUDAmmo();
}

void AWeapon::AddAmmo(const int32 AmmoToAdd)
{
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	SetHUDAmmo();
	ClientAddAmmo(AmmoToAdd);
}

void AWeapon::ClientAddAmmo_Implementation(const int32 AmmoToAdd)
{
	if (HasAuthority())
	{
		return;
	}
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombatComponent() && IsFull())
	{
		BlasterOwnerCharacter->GetCombatComponent()->JumpToShotgunEnd();
	}
}

void AWeapon::HandleUpdateWeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		HandleWeaponStateEquipped();
		break;
	case EWeaponState::EWS_EquippedSecondary:
		HandleWeaponStateEquippedSecondary();
		break;
	case EWeaponState::EWS_Dropped:
		HandleWeaponStateDropped();
		break;
	default:
		break;
	}
}

void AWeapon::HandleWeaponStateEquipped()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}
	EnableCustomDepth(false);

	if (bUseServerSideRewind && HasAuthority() && GetBlasterOwnerController() && !GetBlasterOwnerController()->HighPingDelegate.IsBound())
	{
		BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
	}
}

void AWeapon::HandleWeaponStateEquippedSecondary()
{
	ShowPickupWidget(false);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (WeaponType == EWeaponType::EWT_SubMachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	}

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
	WeaponMesh->MarkRenderStateDirty();

	if (GetBlasterOwnerController() && GetBlasterOwnerController()->HighPingDelegate.IsBound())
	{
		BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
	}
}

void AWeapon::HandleWeaponStateDropped()
{
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	if (GetBlasterOwnerController() && GetBlasterOwnerController()->HighPingDelegate.IsBound())
	{
		BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
	}
}

ABlasterCharacter* AWeapon::GetBlasterOwnerCharacter() const
{
	return BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
}

ABlasterPlayerController* AWeapon::GetBlasterOwnerController()
{
	if (BlasterOwnerController == nullptr)
	{
		if (GetBlasterOwnerCharacter())
		{
			BlasterOwnerController = Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller);
		}
	}
	return BlasterOwnerController;
}

// Server
void AWeapon::SetWeaponState(EWeaponState aState)
{
	WeaponState = aState;

	HandleUpdateWeaponState();
}

bool AWeapon::IsEmpty() const
{
	return Ammo <= 0;
}

bool AWeapon::IsFull() const
{
	return Ammo >= MagCapacity;
}

void AWeapon::OnPingTooHigh(const bool bPingTooHigh)
{
	bUseServerSideRewind = !bPingTooHigh;
}

// Client Only
// ReSharper disable once CppMemberFunctionMayBeConst
void AWeapon::OnRep_WeaponState()
{
	HandleUpdateWeaponState();
}

void AWeapon::ShowPickupWidget(bool bShowWidget) const
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(bShowWidget);
	}
}

void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}

	if (const auto AmmoEjectSocket = GetWeaponMesh()->GetSocketByName(FName("AmmoEject")); AmmoEjectSocket && CasingClass)
	{
		const auto SocketTransform = AmmoEjectSocket->GetSocketTransform(GetWeaponMesh());

		if (const auto World = GetWorld())
		{
			World->SpawnActor<ACasing>(CasingClass, SocketTransform.GetLocation(), SocketTransform.GetRotation().Rotator());
		}
	}
	SpendRound();
}

void AWeapon::Drop()
{
	SetWeaponState(EWeaponState::EWS_Dropped);
	const FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	GetWeaponMesh()->DetachFromComponent(DetachRules);
	SetOwner(nullptr);
}

void AWeapon::SetOwner(AActor* NewOwner)
{
	Super::SetOwner(NewOwner);

	BlasterOwnerCharacter = Cast<ABlasterCharacter>(NewOwner);
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller);
		if (BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
	else
	{
		BlasterOwnerController = nullptr;
	}
}

void AWeapon::SetHUDAmmo() const
{
	if (BlasterOwnerController)
	{
		BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
	}
}
