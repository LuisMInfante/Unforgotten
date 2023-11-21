// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "NiagaraSystemInstanceController.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"


AProjectileRocket::AProjectileRocket() 
{
    RocketMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket MEsh"));
    RocketMesh->SetupAttachment(RootComponent);
    RocketMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AProjectileRocket::BeginPlay() 
{
    Super::BeginPlay();

    CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);

    if (RocketTrailSystem)
    {
        RocketTrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            RocketTrailSystem,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition,
            false
        );
    }
    if (ProjectileLoop && LoopingSoundAttenuation)
    {
        ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
            ProjectileLoop,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            EAttachLocation::KeepWorldPosition,
            false,
            1.f,
            1.f,
            0.f,
            LoopingSoundAttenuation,
            (USoundConcurrency*)nullptr,
            false
        );
    }
}


void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) 
{
    APawn* FiringPawn = GetInstigator();
    if (FiringPawn)
    {
        AController* FiringController = FiringPawn->GetController();
        if (FiringController)
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(
                this, // World Context Object
                Damage, // Base Damage
                MinimumBlastDamage,
                GetActorLocation(), // Origin
                InnerBlastRadius,
                OuterBlastRadius,
                1.f, // Damage Falloff
                UDamageType::StaticClass(), // DamageType Class
                TArray<AActor*>(), // Ignored Actors
                this, // Damage Causer
                FiringController // Instigator controller
            );
        }
    }

    // Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
    GetWorldTimerManager().SetTimer(
        DestroyTimer,
        this,
        &AProjectileRocket::DestroyTimerEnded,
        DestroyTime
    );

    if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
    if (RocketMesh)
    {
        RocketMesh->SetVisibility(false);
    }
    if (CollisionBox)
    {
        CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    if (RocketTrailSystemComponent && RocketTrailSystemComponent->GetSystemInstanceController())
    {
        RocketTrailSystemComponent->GetSystemInstanceController()->Deactivate();
    }
    if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
    {
        ProjectileLoopComponent->Stop();
    }
}

void AProjectileRocket::DestroyTimerEnded() 
{
    Destroy();
}

void AProjectileRocket::Destroyed() 
{

}
