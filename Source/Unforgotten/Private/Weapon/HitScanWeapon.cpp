// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnforgottenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapon/WeaponTypes.h"
#include "DrawDebugHelpers.h"
#include "Enemies/Enemy.h"
#include "Interfaces/BulletHitInterface.h"

void AHitScanWeapon::Fire(const FVector& HitTarget) 
{
    Super::Fire(HitTarget);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    AController* InstigatorController = OwnerPawn->GetController();

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket && InstigatorController)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector Start = SocketTransform.GetLocation();
        FHitResult FireHit;
        WeaponTraceHit(Start, HitTarget, FireHit);

        AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(FireHit.GetActor());
        if (UnforgottenCharacter)
        {
            UGameplayStatics::ApplyDamage(
                UnforgottenCharacter,
                Damage,
                InstigatorController,
                this,
                UDamageType::StaticClass()
            );
        }

        AEnemy* HitEnemy = Cast<AEnemy>(FireHit.GetActor());
        if (HitEnemy)
		{
			IBulletHitInterface* BulletHitInterface = Cast<IBulletHitInterface>(FireHit.GetActor());
			if (BulletHitInterface)
			{
				BulletHitInterface->BulletHit_Implementation(FireHit);
			}

			if (HitEnemy)
			{
				UGameplayStatics::ApplyDamage(
					FireHit.GetActor(),
					Damage,
                    InstigatorController,
                    this,
                    UDamageType::StaticClass()
				);
		    }
        }

        if (ImpactParticles)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                ImpactParticles,
                FireHit.ImpactPoint,
                FireHit.ImpactNormal.Rotation()
            );
        }

        if (HitSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                HitSound,
                FireHit.ImpactPoint
            );
        }        

        if (MuzzleFlash)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                GetWorld(),
                MuzzleFlash,
                SocketTransform
            );
        }

        if (FireSound)
        {
            UGameplayStatics::PlaySoundAtLocation(
                this,
                FireSound,
                GetActorLocation()
            );
        }
    }
}

void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit) 
{
    UWorld* World = GetWorld();

    if (World)
    {
        FVector End = bUseScatter ? TraceEndWithScatter(TraceStart, HitTarget) : TraceStart + (HitTarget - TraceStart) * 1.25f;

        World->LineTraceSingleByChannel(
                OutHit,
                TraceStart,
                End,
                ECollisionChannel::ECC_Visibility
            );

            FVector BeamEnd = End;
            if (OutHit.bBlockingHit)
            {
                BeamEnd = OutHit.ImpactPoint;
            }

            if (BeamParticles)
            {
                UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
                    World,
                    BeamParticles,
                    TraceStart,
                    FRotator::ZeroRotator,
                    true
                );

                if (Beam)
                {
                    Beam->SetVectorParameter(FName("Target"), BeamEnd);
                }
            }
    }
}

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) 
{
    FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
    FVector EndLocation = SphereCenter + RandomVector;
    FVector ToEndLocation = EndLocation - TraceStart;

/*
    DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Cyan, true);
    DrawDebugLine(
        GetWorld(), 
        TraceStart, 
        FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()),
        FColor::Emerald,
        true
    );
*/

    return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}
