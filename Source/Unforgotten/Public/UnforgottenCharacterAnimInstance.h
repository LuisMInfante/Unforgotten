// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "UnforgottenTypes/CombatState.h"
#include "UnforgottenCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UNFORGOTTEN_API UUnforgottenCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	// virtual void NativeInitializeAnimation() override;
	// virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class AUnforgottenCharacter* UnforgottenCharacter;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;
};
