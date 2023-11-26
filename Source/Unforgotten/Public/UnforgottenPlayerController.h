// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UnforgottenPlayerController.generated.h"

class UInputMappingContext;

/**
 *
 */
UCLASS()
class UNFORGOTTEN_API AUnforgottenPlayerController : public APlayerController
{
	GENERATED_BODY()
public:

	virtual void Tick(float DeltaTime) override;

	void PollInit();

	void SetHUDHealth(float CurrentHealth, float MaxHealth);

	void SetHUDShields(float CurrentShields, float MaxShields);

	void SetHUDWeaponAmmo(int32 Ammo);
	
	void SetHUDCarriedAmmo(int32 Ammo);
protected:

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;

	// End Actor interface

private:

	class AUnforgottenHUD* UnforgottenHUD;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;
	bool bInitializeCharacterOverlay = false;

	float HUDCurrentHealth = 100.f;
	float HUDMaxHealth = 100.f;
	bool bInitializeHealth = false;
	
	float HUDCurrentShields = 100.f;
	float HUDMaxShields = 100.f;
	bool bInitializeShields = false;
};
