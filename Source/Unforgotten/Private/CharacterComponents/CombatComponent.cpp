// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterComponents/CombatComponent.h"
#include "Weapon/Weapon.h"
#include "UnforgottenCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "UnforgottenPlayerController.h"
#include "HUD/UnforgottenHUD.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshairs(DeltaTime);

}


void UCombatComponent::EquipWeapon(class AWeapon* WeaponToEquip) 
{
	if(!Character || !WeaponToEquip) return;

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	// const USkeletalMeshSocket* RightHandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	const USkeletalMeshSocket* RightHandSocket = Character->GetMesh1P()->GetSocketByName(FName("RightHandSocket"));

	if(RightHandSocket)
	{
		// RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
		RightHandSocket->AttachActor(EquippedWeapon, Character->GetMesh1P());
		Character->SetHasRifle(true);
	}

	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->ShowPickupWidget(false);
}

void UCombatComponent::FireButtonPressed(bool bPressed) 
{
	bFireButtonPressed = bPressed;

	if(!EquippedWeapon) return;

	if(Character && bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshair(HitResult);
		Character->PlayFireMontage(false); // switch for bIsAiming later
		EquippedWeapon->Fire(HitTarget);
	}
}

void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult) 
{
	FVector2D ViewportSize;

	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Find the center of the screen (screen space)
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	// Translate to 3D world space
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0), 
		CrosshairLocation, 
		CrosshairWorldPosition, 
		CrosshairWorldDirection
	);

	// Line Trace
	if(bScreenToWorld)
	{
		FVector StartLocation = CrosshairWorldPosition; // start at center of screen
		FVector EndLocation = StartLocation + CrosshairWorldDirection * TRACE_LENGTH; // projecting away from screen X units

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			StartLocation,
			EndLocation,
			ECollisionChannel::ECC_Visibility
		);

		// Set impact point if we don't hit anything
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = EndLocation;
			HitTarget = EndLocation;
		}
		else // We hit something in range
		{
			HitTarget = TraceHitResult.ImpactPoint;

			// DrawDebugSphere(
			// 	GetWorld(),
			// 	TraceHitResult.ImpactPoint,
			// 	12.f,
			// 	12,
			// 	FColor::Red,
			// 	false
			// );
		}
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime) 
{
	if (!Character || !Character->Controller) return;

	Controller = !Controller ? Cast<AUnforgottenPlayerController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = !HUD ? Cast<AUnforgottenHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			FHUDPackage HUDPackage;
			
			if (EquippedWeapon)
			{
				HUDPackage.CenterCrosshair = EquippedWeapon->CenterCrosshair;
				HUDPackage.LeftCrosshair = EquippedWeapon->LeftCrosshair;
				HUDPackage.RightCrosshair = EquippedWeapon->RightCrosshair;
				HUDPackage.TopCrosshair = EquippedWeapon->TopCrosshair;
				HUDPackage.BottomCrosshair = EquippedWeapon->BottomCrosshair;
			}
			else
			{
				HUDPackage.CenterCrosshair = nullptr;
				HUDPackage.LeftCrosshair = nullptr;
				HUDPackage.RightCrosshair = nullptr;
				HUDPackage.TopCrosshair = nullptr;
				HUDPackage.BottomCrosshair = nullptr;
			}

			// Calculate crosshair spread
			// [0, MaxMovementSpeed] -> [0, 1]
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed); // might calculate crouching later
			FVector2D MappedVelocityRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			CrosshairVelocityMapped = FMath::GetMappedRangeValueClamped(WalkSpeedRange, MappedVelocityRange, Velocity.Size());
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirMapped = FMath::FInterpTo(CrosshairInAirMapped, 2.25, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirMapped = FMath::FInterpTo(CrosshairInAirMapped, 0, DeltaTime, 30.f);
			}

			HUDPackage.CrosshairSpread = CrosshairVelocityMapped + CrosshairInAirMapped;

			HUD->SetHUDPackage(HUDPackage);
		}
	}

}
