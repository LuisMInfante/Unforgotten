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

		UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	}

	UnforgottenHUD = Cast<AUnforgottenHUD>(GetHUD());
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
}
