// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnforgottenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

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
            }
        }
    }
}
