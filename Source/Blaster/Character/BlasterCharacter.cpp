// Fill out your copyright notice in the Description page of Project Settings.

#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"

// Sets default values
ABlasterCharacter::ABlasterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	Combat->SetIsReplicated(true);

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("Buff"));
	Buff->SetIsReplicated(true);

	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetCharacterMovement()->RotationRate.Yaw = 850.f;
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("AttachedGrenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), TEXT("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	/************************************/
	/* Hit Boxes for Server Side Rewind */
	/************************************/

	head = CreateHitBox(FName("head"));
	pelvis = CreateHitBox(FName("pelvis"));
	spine_02 = CreateHitBox(FName("spine_02"));
	spine_03 = CreateHitBox(FName("spine_03"));
	upperarm_l = CreateHitBox(FName("upperarm_l"));
	upperarm_r = CreateHitBox(FName("upperarm_r"));
	lowerarm_l = CreateHitBox(FName("lowerarm_l"));
	lowerarm_r = CreateHitBox(FName("lowerarm_r"));
	hand_l = CreateHitBox(FName("hand_l"));
	hand_r = CreateHitBox(FName("hand_r"));
	backpack = CreateHitBox(FName("backpack"));
	blanket = CreateHitBox(FName("backpack"), FName("blanket"));
	thigh_l = CreateHitBox(FName("thigh_l"));
	thigh_r = CreateHitBox(FName("thigh_r"));
	calf_l = CreateHitBox(FName("calf_l"));
	calf_r = CreateHitBox(FName("calf_r"));
	foot_l = CreateHitBox(FName("foot_l"));
	foot_r = CreateHitBox(FName("foot_r"));
}

void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, LastHitLocation);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimProxiesTurn();
	TimeSinceLastMovementReplication = 0.f;
}

void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon)
	{
		if (Weapon->bDestroyWeapon)
		{
			Weapon->Destroy();
		}
		else
		{
			Weapon->Drop();
		}
	}
}

void ABlasterCharacter::EliminationServer()
{
	if (Combat)
	{
		DropOrDestroyWeapon(Combat->EquippedWeapon);
		DropOrDestroyWeapon(Combat->SecondaryWeapon);
	}
	MulticastElimination();
	GetWorldTimerManager().SetTimer(EliminationTimer, this, &ABlasterCharacter::EliminationTimerFinished, EliminationDelay);
}

void ABlasterCharacter::MulticastElimination_Implementation()
{
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0);
	}
	bEliminated = true;
	PlayElimMontage();

	// Start dissolve effect
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// Disable character movement
	GetCharacterMovement()->DisableMovement(); // w-a-s-d
	GetCharacterMovement()->StopMovementImmediately(); // disable rotation
	bDisableGameplay = true;
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// Disable collision
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Spawn Elimination Bot
	if (EliminationBotEffect)
	{
		const FVector EliminationBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		EliminationBotComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), EliminationBotEffect, EliminationBotSpawnPoint, GetActorRotation());

		if (EliminationBotSound)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(), EliminationBotSound, EliminationBotSpawnPoint, GetActorRotation());
		}
	}

	// Hide sniper scope if we were aiming with a sniper weapon and get eliminated
	if (IsLocallyControlled() && Combat && Combat->bAiming && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		ShowSniperScopeWidget(false);
	}
}

void ABlasterCharacter::UpdateHUD()
{
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
		if (Combat)
		{
			BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
			BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon ? Combat->EquippedWeapon->GetAmmo() : 0);
			BlasterPlayerController->SetHUDGrenades(Combat->GetGrenades());
		}
		BlasterPlayerController->HideDeathMessage();
	}
}

void ABlasterCharacter::SpawnDefaultWeapon() const
{
	// Return only if we are the server
	UWorld* World = GetWorld();
	if (const ABlasterGameMode* BlasterGameMode = World->GetAuthGameMode<ABlasterGameMode>(); World && BlasterGameMode && !bEliminated && DefaultWeaponClass)
	{
		// Spawn default weapon
		if (AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass))
		{
			Combat->EquipWeapon(StartingWeapon);
			StartingWeapon->bDestroyWeapon = true;
		}
	}
}

void ABlasterCharacter::PollInit()
{
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			BlasterPlayerState->AddToDefeats(0); // Init Defeats
			BlasterPlayerState->AddToScore(0.f); // Init Score
		}
	}
}

void ABlasterCharacter::Heal(const float HealAmount)
{
	if (Health < MaxHealth)
	{
		Health = FMath::Clamp(Health + HealAmount, 0.f, MaxHealth);
		UpdateHUD();
	}
}

void ABlasterCharacter::ReplenishShield(const float ShieldAmount)
{
	if (Shield < MaxShield)
	{
		Shield = FMath::Clamp(Shield + ShieldAmount, 0.f, MaxShield);
		UpdateHUD();
	}
}

void ABlasterCharacter::Destroyed()
{
	Super::Destroyed();

	if (EliminationBotComponent)
	{
		EliminationBotComponent->DestroyComponent();
	}

	const ABlasterGameMode* BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress && Combat && Combat->EquippedWeapon)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

UBoxComponent* ABlasterCharacter::CreateHitBox(FName SocketName)
{
	return CreateHitBox(SocketName, SocketName);
}

UBoxComponent* ABlasterCharacter::CreateHitBox(const FName SocketName, FName BoxName)
{
	UBoxComponent* HitBox = CreateDefaultSubobject<UBoxComponent>(BoxName);
	HitBox->SetupAttachment(GetMesh(), SocketName);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitCollisionBoxes.Add(BoxName, HitBox);
	return HitBox;
}

void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();
	UpdateHUD();
	if (HasAuthority())
	{
		OnTakePointDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
		OnTakeRadialDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamageRadial);
	}

	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

void ABlasterCharacter::RotateInPlace(const float DeltaTime)
{
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	if (GetLocalRole() > ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch();
	}
}

void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	HideCameraIfCharacterClose();
	PollInit();
}

void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::ThrowGrenadeButtonPressed);

	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp);
}

void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}

	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (GetController())
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(GetController());
		}
	}
}

void ABlasterCharacter::PlayFireMontage(bool aIsAiming) const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || FireWeaponMontage == nullptr)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		const FName SectionName = aIsAiming ? FName("RifleAim") : FName("RifleHip");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage() const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || ReloadMontage == nullptr)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(ReloadMontage);

		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubMachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		default:
			SectionName = FName("Rifle");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); ElimMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage() const
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance(); ThrowGrenadeMontage && AnimInstance)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage() const
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr || HitReactMontage == nullptr)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Play(HitReactMontage);
		const FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, class AController* InstigatedBy, FVector HitLocation, class UPrimitiveComponent* FHitComponent, FName BoneName, FVector ShotFromDirection, const class UDamageType* DamageType, AActor* DamageCauser)
{
	ReceiveDamageGeneric(Damage, HitLocation, InstigatedBy);
}

void ABlasterCharacter::ReceiveDamageRadial(AActor* DamagedActor, float Damage, const UDamageType* DamageType, FVector Origin, FHitResult HitInfo, AController* InstigatedBy, AActor* DamageCauser)
{
	ReceiveDamageGeneric(Damage, FVector::ZeroVector, InstigatedBy);
}

void ABlasterCharacter::ReceiveDamageGeneric(const float Damage, const FVector HitLocation, AController* InstigatedBy)
{
	if (bEliminated)
	{
		return;
	}

	float DamageToHealth = Damage;
	if (Shield > 0.f)
	{
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		else
		{
			DamageToHealth = Damage - Shield;
			Shield = 0.f;
		}
	}

	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);
	UpdateHUD();
	PlayHitReactMontage();

	// Will be replicated on client and calls OnRep_HitLocation
	if (HitLocation != FVector::ZeroVector)
	{
		LastHitLocation = HitLocation;
		OnRep_LastHitLocation(); // execute client code on server too.
	}

	if (Health == 0.f)
	{
		if (ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
		{
			const auto AttackerController = Cast<ABlasterPlayerController>(InstigatedBy);
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController, AttackerController);
		}
	}
}

void ABlasterCharacter::MoveForward(float aValue)
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Controller != nullptr && aValue != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		AddMovementInput(Direction, aValue);
	}
}

void ABlasterCharacter::MoveRight(float aValue)
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Controller != nullptr && aValue != 0.f)
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector RightDirection(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		AddMovementInput(RightDirection, aValue);
	}
}

void ABlasterCharacter::Turn(float aValue)
{
	AddControllerYawInput(aValue);
}

void ABlasterCharacter::LookUp(float aValue)
{
	AddControllerPitchInput(aValue);
}

void ABlasterCharacter::EquipButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		ServerEquipButtonPressed();
	}
}

void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

void ABlasterCharacter::CrouchButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Crouch();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst (const function cannot be used in binding)
void ABlasterCharacter::ReloadButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->Reload();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst (const function cannot be used in binding)
void ABlasterCharacter::AimButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst (const function cannot be used in binding)
void ABlasterCharacter::AimButtonReleased()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->SetAiming(false);
	}
}

void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// map pitch from [270, 360) to [-90, 0)
		const FVector2D InRange(270.f, 360.f);
		const FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

float ABlasterCharacter::CalculateSpeed() const
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	return Velocity.Size();
}

void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr)
	{
		return;
	}
	const float Speed = CalculateSpeed();
	const bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir) // standing still, not jumping
	{
		bRotateRootBone = true;
		const FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		const FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw;
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			Interp_AO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true;
		TurnInPlace(DeltaTime);
	}
	if (Speed > 0.f || bIsInAir) // running, or jumping
	{
		bRotateRootBone = false;
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
	}

	CalculateAO_Pitch();
}

void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr)
	{
		return;
	}

	bRotateRootBone = false;
	const float Speed = CalculateSpeed();
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right;
		}
		else if (ProxyYaw < TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left;
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;
}

void ABlasterCharacter::Jump()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst (const function cannot be used in binding)
void ABlasterCharacter::FireButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst (const function cannot be used in binding)
void ABlasterCharacter::FireButtonReleased()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::ThrowGrenadeButtonPressed()
{
	if (bDisableGameplay)
	{
		return;
	}

	if (Combat)
	{
		Combat->ThrowGrenade();
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		Interp_AO_Yaw = FMath::FInterpTo(Interp_AO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = Interp_AO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose() const
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health(const float LastHealth)
{
	UpdateHUD();

	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(const float LastShield)
{
	UpdateHUD();
}

void ABlasterCharacter::OnRep_LastHitLocation() const
{
	if (ImpactPlayerParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactPlayerParticles, LastHitLocation);
	}

	if (ImpactPlayerSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactPlayerSound, LastHitLocation);
	}
}

void ABlasterCharacter::EliminationTimerFinished()
{
	// Called only on server
	if (const auto BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>())
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

void ABlasterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	if (DissolveCurve && DissolveTimeline)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// Called only on the server, hide previous weapon pickup widget
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon) const
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

bool ABlasterCharacter::IsWeaponEquipped() const
{
	return Combat && Combat->EquippedWeapon != nullptr;
}

bool ABlasterCharacter::IsAiming() const
{
	return Combat && Combat->bAiming;
}

FVector ABlasterCharacter::GetHitTarget() const
{
	if (Combat == nullptr)
	{
		return FVector();
	}

	return Combat->HitTarget;
}

AWeapon* ABlasterCharacter::GetEquippedWeapon() const
{
	if (Combat == nullptr)
	{
		return nullptr;
	}

	return Combat->EquippedWeapon;
}

ECombatState ABlasterCharacter::GetCombatState()
{
	if (Combat == nullptr)
	{
		return ECombatState::ECS_MAX;
	}

	return Combat->CombatState;
}

bool ABlasterCharacter::IsLocallyReloading() const
{
	if (Combat == nullptr)
	{
		return false;
	}

	return Combat->bLocallyReloading;
}
