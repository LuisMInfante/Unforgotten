// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UnforgottenHUD.generated.h"

/**
 * 
 */
UCLASS()
class UNFORGOTTEN_API AUnforgottenHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	virtual void DrawHUD() override;
};
