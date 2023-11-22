// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class UNFORGOTTEN_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	virtual void Fire(const FVector& HitTarget) override;	

protected:

	FVector TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget);

private:

	UPROPERTY(EditAnywhere)
	float Damage = 10.f;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSound;

	// Trace End with Scatter

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 100.f;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;
};
