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
        FVector End = Start + (HitTarget - Start) * 1.25f; // go past the hit location to guarentee hit

        FHitResult FireHit;
        UWorld* World = GetWorld();
        if (World)
        {
            World->LineTraceSingleByChannel(
                FireHit,
                Start,
                End,
                ECollisionChannel::ECC_Visibility
            );

            FVector BeamEnd = End; // End of beam if not hitting anything

            if (FireHit.bBlockingHit)
            {
                BeamEnd = FireHit.ImpactPoint;
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

                if (ImpactParticles)
                {
                    UGameplayStatics::SpawnEmitterAtLocation(
                        World,
                        ImpactParticles,
                        FireHit.ImpactPoint,
                        FireHit.ImpactNormal.Rotation()
                    );
                }

                if (BeamParticles)
                {
                    UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
                        World,
                        BeamParticles,
                        SocketTransform
                    );

                    if (Beam)
                    {
                        Beam->SetVectorParameter(FName("Target"), BeamEnd);
                    }
                }

                if (HitSound)
                {
                    UGameplayStatics::PlaySoundAtLocation(
                        this,
                        HitSound,
                        FireHit.ImpactPoint
                    );
                }
            }
        }

        if (MuzzleFlash)
        {
            UGameplayStatics::SpawnEmitterAtLocation(
                World,
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

FVector AHitScanWeapon::TraceEndWithScatter(const FVector& TraceStart, const FVector& HitTarget) 
{
    FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
    FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
    FVector RandomVector = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
    FVector EndLocation = SphereCenter + RandomVector;
    FVector ToEndLocation = EndLocation - TraceStart;

    DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
    DrawDebugSphere(GetWorld(), EndLocation, 4.f, 12, FColor::Cyan, true);
    DrawDebugLine(
        GetWorld(), 
        TraceStart, 
        FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size()),
        FColor::Emerald,
        true
    );

    return FVector(TraceStart + ToEndLocation * TRACE_LENGTH / ToEndLocation.Size());
}
