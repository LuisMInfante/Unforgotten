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

	void SetHUDHealth(float CurrentHealth, float MaxHealth);

	void SetHUDWeaponAmmo(int32 Ammo);
	
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
};
