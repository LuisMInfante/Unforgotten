// Copyright Epic Games, Inc. All Rights Reserved.


#include "UnforgottenPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "HUD/UnforgottenHUD.h"
#include "HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void AUnforgottenPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);

		// UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	}

	UnforgottenHUD = Cast<AUnforgottenHUD>(GetHUD());
}

void AUnforgottenPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// PollInit();
}

void AUnforgottenPlayerController::PollInit()
{
	// if (!CharacterOverlay)
	// {
	// 	if (UnforgottenHUD && UnforgottenHUD->CharacterOverlay)
	// 	{
	// 		CharacterOverlay = UnforgottenHUD->CharacterOverlay;
	// 		if (CharacterOverlay)
	// 		{
	// 			SetHUDHealth(HUDCurrentHealth, HUDMaxHealth);
	// 			SetHUDShields(HUDCurrentShields, HUDMaxShields);
	// 		}
	// 	}
	// }
}

void AUnforgottenPlayerController::SetHUDHealth(float CurrentHealth, float MaxHealth) 
{
	// if hud is null then cast to hud
	UnforgottenHUD = !UnforgottenHUD ? Cast<AUnforgottenHUD>(GetHUD()) : UnforgottenHUD;

	bool bHUDIsValid = UnforgottenHUD && 
					   UnforgottenHUD->CharacterOverlay && 
					   UnforgottenHUD->CharacterOverlay->HealthBar && 
					   UnforgottenHUD->CharacterOverlay->HealthText;

	if(bHUDIsValid)
	{
		const float HealthPercentage = CurrentHealth / MaxHealth;
		UnforgottenHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercentage);
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurrentHealth), FMath::CeilToInt(MaxHealth));
		UnforgottenHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	// else
	// {
	// 	bInitializeCharacterOverlay = true;
	// 	HUDCurrentHealth = CurrentHealth;
	// 	HUDMaxHealth = MaxHealth;
	// }
}

void AUnforgottenPlayerController::SetHUDShields(float CurrentShields, float MaxShields) 
{
	// if hud is null then cast to hud
	UnforgottenHUD = !UnforgottenHUD ? Cast<AUnforgottenHUD>(GetHUD()) : UnforgottenHUD;

	bool bHUDIsValid = UnforgottenHUD && 
					   UnforgottenHUD->CharacterOverlay && 
					   UnforgottenHUD->CharacterOverlay->ShieldBar && 
					   UnforgottenHUD->CharacterOverlay->ShieldText;

	if(bHUDIsValid)
	{
		const float ShieldPercentage = CurrentShields / MaxShields;
		UnforgottenHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercentage);
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(CurrentShields), FMath::CeilToInt(MaxShields));
		UnforgottenHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	// else
	// {
	// 	bInitializeCharacterOverlay = true;
	// 	HUDCurrentShields = CurrentShields;
	// 	HUDMaxShields = MaxShields;
	// }
}

void AUnforgottenPlayerController::SetHUDWeaponAmmo(int32 Ammo) 
{
	// if hud is null then cast to hud
	UnforgottenHUD = !UnforgottenHUD ? Cast<AUnforgottenHUD>(GetHUD()) : UnforgottenHUD;

	bool bHUDIsValid = UnforgottenHUD && 
					   UnforgottenHUD->CharacterOverlay &&  
					   UnforgottenHUD->CharacterOverlay->WeaponAmmoAmount;

	if(bHUDIsValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		UnforgottenHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}


void AUnforgottenPlayerController::SetHUDCarriedAmmo(int32 Ammo) 
{
		// if hud is null then cast to hud
	UnforgottenHUD = !UnforgottenHUD ? Cast<AUnforgottenHUD>(GetHUD()) : UnforgottenHUD;

	bool bHUDIsValid = UnforgottenHUD && 
					   UnforgottenHUD->CharacterOverlay &&  
					   UnforgottenHUD->CharacterOverlay->CarriedAmmoAmount;

	if(bHUDIsValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		UnforgottenHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
