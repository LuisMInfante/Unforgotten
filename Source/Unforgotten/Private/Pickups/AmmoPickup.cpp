// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/AmmoPickup.h"
#include "Unforgotten/Public/UnforgottenCharacter.h"
#include "Unforgotten/Public/CharacterComponents/CombatComponent.h"

void AAmmoPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) 
{
    Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

    AUnforgottenCharacter* UnforgottenCharacter = Cast<AUnforgottenCharacter>(OtherActor);

    if(UnforgottenCharacter)
    {
        UCombatComponent* Combat = UnforgottenCharacter->GetCombatComponent();

        if (Combat)
        {
            Combat->PickupAmmo(WeaponType, AmountOfAmmo);
        }
    }
    
    Destroy();
}
