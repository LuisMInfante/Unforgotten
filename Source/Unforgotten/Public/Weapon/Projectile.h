// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

class UParticleSystem;

UCLASS()
class UNFORGOTTEN_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void Destroyed() override;

	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void StartDestroyTimer();
	void DestroyTimerEnded();
	void SpawnTrailSystem();
	void ExplosionDamage();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere)
	float MinimumBlastDamage = 10.f;
	UPROPERTY(EditAnywhere)
	float InnerBlastRadius = 200.f;
	UPROPERTY(EditAnywhere)
	float OuterBlastRadius = 500.f;

private:

	float ProjectileImpulse;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer;

	class UParticleSystemComponent* TracerComponent;

	FTimerHandle DestroyTimer;

	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;

public:	

	FORCEINLINE float GetDamage() const { return Damage; }
};
