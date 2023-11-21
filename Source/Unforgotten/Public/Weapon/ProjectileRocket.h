// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon/Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 
 */
UCLASS()
class UNFORGOTTEN_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()

public:

	AProjectileRocket();
	virtual void Destroyed() override;

protected:

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;	
	virtual void BeginPlay() override;
	void DestroyTimerEnded();

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* RocketTrailSystem;

	UPROPERTY()
	class UNiagaraComponent* RocketTrailSystemComponent;

	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	UPROPERTY(EditAnywhere)
	float MinimumBlastDamage = 10.f;
	UPROPERTY(EditAnywhere)
	float InnerBlastRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float OuterBlastRadius = 500.f;

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
};
