// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UNFORGOTTEN_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBuffComponent();

	friend class AUnforgottenCharacter;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Heal(float HealAmount, float HealTime);
	void RechargeShields(float ShieldAmount, float RechargeTime);

	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	void SetInitialSpeed(float BaseSpeed, float CrouchSpeed);

	void BuffJump(float BuffJumpVelocity, float BuffTime);
	void SetInitialJumpVelocity(float Velocity);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	void HealingRampUp(float DeltaTime);
	void ShieldRechargeRampUp(float DeltaTime);

private:

	UPROPERTY()
	class AUnforgottenCharacter* Character;

	// Healing
	bool bHealing = false;
	float HealingRate = 0.f;
	float AmountToHeal = 0.f;

	// Shield Recharging
	bool bRecharging = false;
	float ShieldRechargeRate = 0.f;
	float AmountToRecharge = 0.f;

	// Speed buff
	FTimerHandle SpeedBuffTimer;
	void ResetSpeed();
	float InitialBaseSpeed;
	float InitialCrouchSpeed;

	// Jump buff
	FTimerHandle JumpBuffTimer;
	void ResetJump();
	float InitialJumpVelocity;

public:	

};
