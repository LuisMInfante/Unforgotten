// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UnforgottenHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:

	class UTexture2D* CenterCrosshair;
	UTexture2D* LeftCrosshair;
	UTexture2D* RightCrosshair;
	UTexture2D* TopCrosshair;
	UTexture2D* BottomCrosshair;

	float CrosshairSpread;

};
UCLASS()
class UNFORGOTTEN_API AUnforgottenHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	virtual void DrawHUD() override;

private: 

	FHUDPackage HUDPackage;

	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread);

	UPROPERTY(EditAnywhere)
	float MaxCrosshairSpread = 16.f;
	
public:

	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
