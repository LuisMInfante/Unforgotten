// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HUD/UnforgottenHUD.h"
#include "Unforgotten/Public/Weapon/WeaponTypes.h"
#include "Unforgotten/Public/UnforgottenTypes/CombatState.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNFORGOTTEN_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	friend class AUnforgottenCharacter; // *** CHANGE THIS LATER *** (friend gives class FULL access)

	void EquipWeapon(class AWeapon* WeaponToEquip);
	void Reload();
	UFUNCTION(BlueprintCallable)
	void FinishedReloading();
	int32 AmountToReload();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void FireButtonPressed(bool bPressed);

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:

	class AUnforgottenCharacter* Character; 
	class AUnforgottenPlayerController* Controller;
	class AUnforgottenHUD* HUD;
	AWeapon* EquippedWeapon;

	bool bFireButtonPressed;

	FVector HitTarget;

	// HUD/Crosshair
	float CrosshairVelocityMapped;
	float CrosshairInAirMapped;

	FHUDPackage HUDPackage;

	// Full Auto
	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerEnded();
	void Fire();

	bool CanFire();

	// For currently equipped weapon
	UPROPERTY()
	int32 CarriedAmmo;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 StartingAssaultRifleAmmo = 30;
	UPROPERTY(EditAnywhere)
	int32 StartingRocketLauncherAmmo = 4;
		
	void InitializeCarriedAmmo();

	ECombatState CombatState = ECombatState::ECS_Unoccupied;
public:	

		
};
