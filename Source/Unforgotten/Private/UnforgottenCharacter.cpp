// Copyright Epic Games, Inc. All Rights Reserved.

#include "UnforgottenCharacter.h"
#include "UnforgottenProjectile.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "Weapon/Weapon.h"
#include "CharacterComponents/CombatComponent.h"
#include "CharacterComponents/BuffComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Unforgotten/Public/UnforgottenPlayerController.h"
#include "Unforgotten/Public/Weapon/WeaponTypes.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// AUnforgottenCharacter

AUnforgottenCharacter::AUnforgottenCharacter()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));

	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));

}

void AUnforgottenCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UpdateHUDHealth();
	UpdateHUDShields();

	OnTakeAnyDamage.AddDynamic(this, &AUnforgottenCharacter::RecieveDamage);
}

void AUnforgottenCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


}
//////////////////////////////////////////////////////////////////////////// Input

void AUnforgottenCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::Look);

		// Equip
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::Equip);

		// Fire Weapon
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AUnforgottenCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AUnforgottenCharacter::FireButtonReleased);

		// Reload Weapon
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AUnforgottenCharacter::ReloadButtonPressed);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AUnforgottenCharacter::PostInitializeComponents() 
{
	Super::PostInitializeComponents();
	
	if (Combat)
	{
		Combat->Character = this;
	}

	if (Buff)
	{
		Buff->Character = this;
		Buff->SetInitialSpeed(
			GetCharacterMovement()->MaxWalkSpeed, 
			GetCharacterMovement()->MaxWalkSpeedCrouched
		);

		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity);
	}
}


void AUnforgottenCharacter::PlayFireMontage(bool bIsAiming) 
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);

		// Logic for when we implement ADS
		FName SectionName;
		SectionName = bIsAiming ? FName("ADS") : FName("Default");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// May have to put this in Weapon.cpp
void AUnforgottenCharacter::PlayReloadRifleMontage() 
{
	if(!Combat || !Combat->EquippedWeapon) return;

	UAnimInstance* AnimInstance = Combat->EquippedWeapon->GetWeaponMesh()->GetAnimInstance();
	
	if(AnimInstance && ReloadRifleMontage)
	{
		AnimInstance->Montage_Play(ReloadRifleMontage);

		FName SectionName;
		switch (Combat->EquippedWeapon->GetWeaponType())
		{
			case EWeaponType::EWT_AssaultRifle:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_RocketLauncher:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_Pistol:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_SubmachineGun:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_Shotgun:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_SniperRifle:
				SectionName = FName("Rifle");
				break;
			case EWeaponType::EWT_GrenadeLauncher:
				SectionName = FName("Rifle");
				break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AUnforgottenCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AUnforgottenCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AUnforgottenCharacter::Equip() 
{
	if(Combat)
	{
		Combat->EquipWeapon(OverlappingWeapon);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Equip Action Pressed!"));
}

void AUnforgottenCharacter::FireButtonPressed() 
{
	if(Combat)
	{
		Combat->FireButtonPressed(true);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Fire Button Pressed!"));
}


void AUnforgottenCharacter::FireButtonReleased() 
{
	if(Combat)
	{
		Combat->FireButtonPressed(false);
	}

	// UE_LOG(LogTemp, Warning, TEXT("Fire Button Released!"));
}

void AUnforgottenCharacter::ReloadButtonPressed() 
{
	if (Combat)
	{
		Combat->Reload();
	}
}

void AUnforgottenCharacter::RecieveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser) 
{
	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.f, MaxHealth);
	UpdateHUDHealth();
}

void AUnforgottenCharacter::UpdateHUDHealth()
{
	UnforgottenPlayerController = !UnforgottenPlayerController ? Cast<AUnforgottenPlayerController>(Controller) : UnforgottenPlayerController;

	if (UnforgottenPlayerController)
	{
		UnforgottenPlayerController->SetHUDHealth(CurrentHealth, MaxHealth);
	}
}

void AUnforgottenCharacter::UpdateHUDShields()
{
	UnforgottenPlayerController = !UnforgottenPlayerController ? Cast<AUnforgottenPlayerController>(Controller) : UnforgottenPlayerController;

	if (UnforgottenPlayerController)
	{
		UnforgottenPlayerController->SetHUDShields(CurrentShields, MaxShields);
	}
}

void AUnforgottenCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AUnforgottenCharacter::GetHasRifle()
{
	return bHasRifle;
}

ECombatState AUnforgottenCharacter::GetCombatState() const
{
	if (!Combat) return ECombatState::ECS_MAX;
	return Combat->CombatState;
}
