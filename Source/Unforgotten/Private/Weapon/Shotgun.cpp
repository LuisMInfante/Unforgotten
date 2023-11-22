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

        // Randomly generate spread based on number of pellets
        for (uint32 i = 0; i < NumberOfShells; i++)
        {
            FVector End = TraceEndWithScatter(Start, HitTarget);
        }
    }
}
