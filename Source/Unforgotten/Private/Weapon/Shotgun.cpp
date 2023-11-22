// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon/Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "UnforgottenCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

void AShotgun::Fire(const FVector& HitTarget) 
{
    AWeapon::Fire(HitTarget);

    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;
    AController* InstigatorController = OwnerPawn->GetController();

    const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
    if (MuzzleFlashSocket && InstigatorController)
    {
        FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
        FVector Start = SocketTransform.GetLocation();

        TMap<AUnforgottenCharacter*, uint32> HitMap;
        // Randomly generate spread based on number of pellets
        for (uint32 i = 0; i < NumberOfShells; i++)
        {
            FHitResult FireHit;
            WeaponTraceHit(Start, HitTarget, FireHit);

            AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(FireHit.GetActor());
            if (UnforgottenCharacter)
            {   
                if (HitMap.Contains(UnforgottenCharacter))
                {
                    HitMap[UnforgottenCharacter]++; // add number of hits if character is in our map
                }
                else
                {
                    HitMap.Emplace(UnforgottenCharacter, 1); // add new damaged character to map with a confirmed hit
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
                    FireHit.ImpactPoint,
                    .5f,
                    FMath::FRandRange(-0.5f, 0.5f)
                );
            }         
        }

        // Iterate through map can apply damage to each actor according to confirmed hits
        for (auto HitPair : HitMap)
        {
            if (HitPair.Key && InstigatorController)
            {
                UGameplayStatics::ApplyDamage(
                    HitPair.Key,
                    Damage * HitPair.Value,
                    InstigatorController,
                    this,
                    UDamageType::StaticClass()
                );   
            }
        }
    }
}
