// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/ShieldPickup.h"
#include "Unforgotten/Public/UnforgottenCharacter.h"
#include "Unforgotten/Public/CharacterComponents/BuffComponent.h"

void AShieldPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent,
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
        UBuffComponent* Buff = UnforgottenCharacter->GetBuff();

        if (Buff)
        {
            Buff->RechargeShields(ShieldRechargeAmount, ShieldRechargeTime);
        }
    }
    
    Destroy();
}